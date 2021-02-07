#include "inc.h"
template<uint8_t key_len> struct HG {
	int m, win, t, d, n;
	const double c = 1/1.08;

	uint32_t **id, ***s;
	BOBHash32 *hash;

	int Space() {
		return (32 + t * 32) * (n * d) / 8;
	}

	HG(int _m, int _win, int _t, int _d) : m(_m), win(_win), t(_t), d(_d) {
		t = 2 * t + 2;
		n = m / d;

		id = new uint32_t*[n];
		s = new uint32_t**[n];

		rep2 (i, 0, n) {
			id[i] = new uint32_t[d];
			s[i] = new uint32_t*[d];
			rep2 (j, 0, d) {
                id[i][j] = 0;
                s[i][j] = new uint32_t[t];
                rep2 (k, 0, t) s[i][j][k] = 0;
            }
		}
		hash = new BOBHash32(uint8_t(rand() % MAX_PRIME32));
	}

	double counter_query(int idx, int pos, int cur, double rate) {
        double res = 0;
        rep2 (i, 0, t-2) {
			res += s[idx][pos][cur];
			--cur;
			if (cur < 0) cur += t;
		}
        res += s[idx][pos][cur] * rate;
        return res;
	}

	double counter_halfquery(int idx, int pos, int cur, double rate) {
        double res = 0;
        int nt = (t - 2) / 2 + 2;
        rep2 (i, 0, nt-2) {
			res += s[idx][pos][cur];
			--cur;
			if (cur < 0) cur += t;
		}
        res += s[idx][pos][cur] * rate;
        return res;
	}

	void counter_sub(int idx, int pos, int cur) {
        rep2 (i, 0, t) {
			if (s[idx][pos][cur]) {
                s[idx][pos][cur]--;
                return;
            }
			--cur;
			if (cur < 0) cur += t;
		}
	}

	void clear(int idx, int cur) {
        cur++;
        if (cur >= t) cur -= t;
		rep2 (i, 0, d) s[idx][i][cur] = 0;
	}

	void insert(uint8_t *key, uint64_t clock) {
        int cur = (clock / (win / ((t-2) / 2))) % t;
		int idx = hash->run((char *)key, key_len) % n;
        double rate = 1 - 1.0 * (clock % (win / ((t-2) / 2))) / (win / ((t-2) / 2));
		
        for (int nidx = idx, i = 0; i < 8; i++) {
			clear(nidx, cur);
			++nidx;
			if (nidx >= n) nidx -= n;
		}

		uint32_t tuple = *(uint32_t *)key;

		rep2 (i, 0, d) if (id[idx][i] == tuple) {
            s[idx][i][cur]++;
			return;
		}

		double * sum = new double[d];

		rep2 (i, 0, d) sum[i] = counter_query(idx, i, cur, rate);

		int argmin = -1; double minsum = 1e9;
		rep2 (i, 0, d) if (sum[i] < minsum) minsum = sum[i], argmin = i;

		if (minsum == 0) {
            rep2 (i, 0, t) s[idx][argmin][i] = 0;
            s[idx][argmin][cur]++;
			id[idx][argmin] = tuple;
		} else if (rand() < pow(c, sum[argmin]) * RAND_MAX) {
            counter_sub(idx, argmin, cur);
			if (counter_query(idx, argmin, cur, rate) < 1) {
                rep2 (i, 0, t) s[idx][argmin][i] = 0;
                id[idx][argmin] = 0;
            }
		}
	}

	vector<pii> heavychange(int K, uint64_t clock) {
        int cur = (clock / (win / ((t-2) / 2))) % t;
        double rate = 1 - 1.0 * (clock % (win / ((t-2) / 2))) / (win / ((t-2) / 2));
        vector<pii> hc;
		rep2 (i, 0, n) rep2 (j, 0, d) {
            hc.pb(mp(id[i][j], abs(2 * counter_halfquery(i, j, cur, rate) - counter_query(i, j, cur, rate))));
        }
        sort(All(hc), cmp);
        hc.resize(K);
		return hc;
	}
};
