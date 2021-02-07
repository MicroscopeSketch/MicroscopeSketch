#include "inc.h"
template<uint8_t key_len> struct MicroSketch_CU {
	int win, m, n, t, size_k, hnum, log_base;
	int lst = -1;

	uint32_t ***a, **b, **s;
	BOBHash32 **hash;

	int Space() {
		return (t*size_k + 32-size_k + ceil(5 - log(log_base)/log(2))) * n * hnum / 8;
	}

    void counter_add(int h, int idx, int cur) {
        ++s[h][idx];
        while (s[h][idx] >> (log_base * b[h][idx])) {
            s[h][idx] -= 1 << (log_base * b[h][idx]);
            if (a[h][idx][cur] == (1 << size_k) - 1) {
                ++b[h][idx];
                a[h][idx][cur] = 1 << (size_k - log_base);
                rep2 (i, 0, t) if (i != cur) {
                    /*if (rd() & ((1 << log_base) - 1) < a[h][idx][i] & ((1 << log_base) - 1))*/ a[h][idx][i] += (1 << log_base) - 1;
					a[h][idx][i] >>= log_base;
                }
            } else ++a[h][idx][cur];
        }
    }

    void counter_convert(int h, int idx, int cur) {
        /*
        if (rd() & ((1 << log_base * b[h][idx]) - 1) >= s[h][idx]) {
            s[h][idx] = 0;
        } else {
            */
            s[h][idx] = 0;
            if (a[h][idx][cur] == (1 << size_k) - 1) {
                ++b[h][idx];
                a[h][idx][cur] = 1 << (size_k - log_base);
                rep2 (i, 0, t) if (i != cur) {
                    /*if (rd() & ((1 << log_base) - 1) < a[h][idx][i] & ((1 << log_base) - 1))*/ a[h][idx][i] += (1 << log_base) - 1;
					a[h][idx][i] >>= log_base;
                }
            } else ++a[h][idx][cur];
    //    }
    }

    void counter_maintain(int h, int idx) {
        while (b[h][idx]) {
            bool mle = 0;
            rep2 (i, 0, t) if (a[h][idx][i] >> (size_k - log_base)) mle = 1;
            if (mle) return;
            --b[h][idx];
            rep2 (i, 0, t) a[h][idx][i] <<= log_base;
        }
    }

	void counter_clear(int h, int idx, int cur) {
		++cur;
		if (cur >= t) cur -= t;
        if (!a[h][idx][cur]) return;
        a[h][idx][cur] = 0;
        counter_maintain(h, idx);
	}
    
	double counter_query(int h, int idx, int cur, double rate) {
        rate = 1;
        double res = s[h][idx];
        rep2 (i, 0, t-2) {
			res += a[h][idx][cur] << (log_base * b[h][idx]);
			--cur;
			if (cur < 0) cur += t;
		}
        res += (a[h][idx][cur] << (log_base * b[h][idx])) * rate;
        return res;
	}

	double counter_query_new(int *hashid, int cur, double rate) {
		double res = 0;
		uint32_t tmp = 2e9;
		rep2 (k, 0, hnum) tmp = min(tmp, s[k][hashid[k]] + (a[k][hashid[k]][cur] << (log_base * b[k][hashid[k]])));
		--cur;
		if (cur < 0) cur += t;
		res += tmp;
		rep2 (i, 0, t-3) {
			tmp = 2e9;
			rep2 (k, 0, hnum) tmp = min(tmp, a[k][hashid[k]][cur] << (log_base * b[k][hashid[k]]));
			res += tmp;
			--cur;
			if (cur < 0) cur += t;
		}
		tmp = 2e9;
		rep2 (k, 0, hnum) tmp = min(tmp, a[k][hashid[k]][cur] << (log_base * b[k][hashid[k]]));
		res += tmp * rate;
		return res;
	}

	MicroSketch_CU(int _m, int _win, int _t, int _size_k, int _hnum, int _log_base) : m(_m), win(_win), t(_t), size_k(_size_k), hnum(_hnum), log_base(_log_base) {
		t += 2;
		n = m;

		a = new uint32_t**[hnum];
        b = new uint32_t*[hnum];
        s = new uint32_t*[hnum];
		rep2 (i, 0, hnum) {
			a[i] = new uint32_t*[n];
            b[i] = new uint32_t[n];
            s[i] = new uint32_t[n];
			rep2 (j, 0, n) {
                a[i][j] = new uint32_t[t];
                s[i][j] = b[i][j] = 0;
                rep2 (k, 0, t) a[i][j][k] = 0;
            }
		}

		hash = new BOBHash32*[hnum];
		rep2 (i, 0, hnum) hash[i] = new BOBHash32(uint8_t(rd() % MAX_PRIME32));
	}

	void insert(uint8_t *key, uint64_t clock) {
        int cur = (clock / (win / (t-2))) % t;
		int hashid[hnum];
		rep2 (i, 0, hnum) hashid[i] = hash[i]->run((char *)key, key_len) % n;
        double rate = 1 - 1.0 * (clock % (win / (t-2))) / (win / (t-2));
        
		
		if (cur != lst) rep2 (k, 0, hnum) rep2 (idx, 0, n) {
            counter_convert(k, idx, lst);
            counter_clear(k, idx, cur);
        }
		lst = cur;

        int tmpsum[hnum], mini = 2e9;
		rep2 (k, 0, hnum) {
			int idx = hashid[k];
            tmpsum[k] = s[k][idx] + (a[k][idx][cur] << (log_base * b[k][idx]));
            mini = min(mini, tmpsum[k]);
		}
        rep2 (k, 0, hnum) if (tmpsum[k] == mini) counter_add(k, hashid[k], cur);
	}

	double query(uint8_t *key, uint64_t clock) {
        int cur = (clock / (win / (t-2))) % t;
		int hashid[hnum];
		rep2 (i, 0, hnum) hashid[i] = hash[i]->run((char *)key, key_len) % n;
        double rate = 1 - 1.0 * (clock % (win / (t-2))) / (win / (t-2));

		return counter_query_new(hashid, cur, rate);
	}
};
