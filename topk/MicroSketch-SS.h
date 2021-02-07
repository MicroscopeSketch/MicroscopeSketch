#include "inc.h"
template<uint8_t key_len> struct MicroSketch_SS {
	int m, win, t, d, size_k, n, log_base;
	const double c = 1/1.08;

	uint32_t **id, **b, **s, ***a;
	BOBHash32 *hash;

	int Space() {
		return (32 + t*size_k + 32-size_k + ceil(5 - log(log_base)/log(2))) * (n * d) / 8;
	}

	MicroSketch_SS(int _m, int _win, int _t, int _d, int _size_k, int _log_base) : m(_m), win(_win), t(_t), d(_d), size_k(_size_k), log_base(_log_base) {
		t += 2;
		n = m / d;

		id = new uint32_t*[n];
		a = new uint32_t**[n];
		b = new uint32_t*[n];
		s = new uint32_t*[n];

		rep2 (i, 0, n) {
			id[i] = new uint32_t[d];
			b[i] = new uint32_t[d];
			s[i] = new uint32_t[d];
			a[i] = new uint32_t*[t];
			rep2 (j, 0, d) {
				id[i][j] = b[i][j] = s[i][j] = 0;
			}
			rep2 (j, 0, t) {
				a[i][j] = new uint32_t[d];
				rep2 (k, 0, d) a[i][j][k] = 0;
			}
		}
		hash = new BOBHash32(uint8_t(rand() % MAX_PRIME32));
	}

    void counter_maintain(int idx, int pos) {
        while (b[idx][pos]) {
            bool mle = 0;
            rep2 (i, 0, t) if (a[idx][i][pos] >> (size_k - log_base)) mle = 1;
            if (mle) return;
            --b[idx][pos];
            rep2 (i, 0, t) a[idx][i][pos] <<= log_base;
        }
    }
	void counter_clear(int idx, int cur, int pos) {
		++cur;
		if (cur >= t) cur -= t;
        if (!a[idx][cur][pos]) return;
        a[idx][cur][pos] = 0;
        counter_maintain(idx, pos);
	}

	void counter_add(int idx, int cur, int pos) {
        ++s[idx][pos];
        while (s[idx][pos] >> (log_base * b[idx][pos])) {
            s[idx][pos] -= 1 << (log_base * b[idx][pos]);
            if (a[idx][cur][pos] == (1 << size_k) - 1) {
                ++b[idx][pos];
                a[idx][cur][pos] = 1 << (size_k - log_base);
                rep2 (i, 0, t) if (i != cur) {
                    if (rand() & ((1 << log_base) - 1) < a[idx][i][pos] & ((1 << log_base) - 1)) a[idx][i][pos] += (1 << log_base) - 1;
					a[idx][i][pos] >>= log_base;
                }
            } else ++a[idx][cur][pos];
        }
	}

	double counter_query(int idx, int cur, int pos, double rate) {
        double res = s[idx][pos];
        rep2 (i, 0, t-2) {
			res += a[idx][cur][pos] << (log_base * b[idx][pos]);
			--cur;
			if (cur < 0) cur += t;
		}
        res += (a[idx][cur][pos] << (log_base * b[idx][pos])) * rate;
        return res;
	}
    void counter_setzero(int idx, int pos) {
        b[idx][pos] = s[idx][pos] = 0;
        rep2 (i, 0, t) a[idx][i][pos] = 0;
    }
	void counter_sub(int idx, int cur, int pos) {
        if (s[idx][pos]) --s[idx][pos];
        else {
            rep2 (i, 0, t) {
				if (a[idx][cur][pos]) {
					--a[idx][cur][pos];
					s[idx][pos] += (1 << (log_base * b[idx][pos])) - 1;
					counter_maintain(idx, pos);
					return;
				}
				--cur;
				if (cur < 0) cur += t;
			}
        }
	}

	void clear(int idx, int cur) {
		rep2 (i, 0, d) counter_clear(idx, cur, i);
	}

	void insert(uint8_t *key, uint64_t clock) {
        int cur = (clock / (win / (t-2))) % t;
		int idx = hash->run((char *)key, key_len) % n;
        double rate = 1 - 1.0 * (clock % (win / (t-2))) / (win / (t-2));
		
        for (int nidx = idx, i = 0; i < 8; i++) {
			clear(nidx, cur);
			++nidx;
			if (nidx >= n) nidx -= n;
		}

		uint32_t tuple = *(uint32_t *)key;

		rep2 (i, 0, d) if (id[idx][i] == tuple) {
            counter_add(idx, cur, i);
			return;
		}

		double * sum = new double[d];


		rep2 (i, 0, d) sum[i] = counter_query(idx, cur, i, rate);

		int argmin = -1; double minsum = 1e9;
		rep2 (i, 0, d) if (sum[i] < minsum) minsum = sum[i], argmin = i;

        id[idx][argmin] = tuple;
        counter_add(idx, cur, argmin);
	}

	vector<pii> topk(int K, uint64_t clock) {
        int cur = (clock / (win / (t-2))) % t;
        double rate = 1 - 1.0 * (clock % (win / (t-2))) / (win / (t-2));
        vector<pii> topk;
		rep2 (i, 0, n) rep2 (j, 0, d) {
            topk.pb(mp(id[i][j], counter_query(i, cur, j, rate)));
        }
        sort(All(topk), cmp);
        topk.resize(K);
		return topk;
	}
};
