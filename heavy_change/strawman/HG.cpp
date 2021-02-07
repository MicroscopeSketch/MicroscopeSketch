#include "../HG.h"
const int M = 6e3, T = 16, D = 8;
const int QK = 500;
const int TRACE_LEN = 16;
const int TUPLE_LEN = 4;
const int WIN = 1e6;
const int MAXINPUT = 1e7;
const int NUM = 3, LIM = 500;
uint32_t addr[MAXINPUT];
uint64_t timestamp[MAXINPUT];
pil strrepA[MAXINPUT], strrepB[MAXINPUT];
map<uint32_t, int> strmapA, strmapB;

uint64_t calc(uint32_t a, uint32_t b) {
	return 1ll * a * 1000000 + b;
}

void load_CAIDA() {
	char CAIDA[100];
	sprintf(CAIDA, "E:\\DataSet\\CAIDA\\formatted00.dat");
	ifstream fin(CAIDA, ios::binary);
    uint8_t key[TRACE_LEN];
    rep2 (pkt, 0, MAXINPUT) {
        fin.read((char *)key, TRACE_LEN);
        memcpy(addr+pkt, key+8, TUPLE_LEN);
//		timestamp[pkt] = calc(*((uint32_t *) key), *((uint32_t *) key+1)); // time-based
		timestamp[pkt] = pkt; // count-based
    }
}

int main() {
    load_CAIDA();
	freopen("output.csv", "w", stdout);
	srand(time(0));
	cout << "HG" << endl;
	for (int mem = 10; mem <= 320; mem <<= 1) {
		strmapA.clear();
		strmapB.clear();
		int M = mem * 1024 * 8 / (32 + (T+2)*32);
		auto sketch = new HG<TUPLE_LEN>(M, WIN, T, D);
		double totare = 0, totrecall = 0;
		int cnt = 0, hA = 0, tA = 0, hB = 0, tB = 0, pkt = 0;
		while (cnt < 100 * (NUM-1)) {
			uint32_t tuple = addr[pkt];

			while (hA != tA && strrepA[hA].se + WIN <= timestamp[pkt]) {
				strrepB[tB++] = strrepA[hA];
				strmapB[strrepA[hA].fi] += 1;
				strmapA[strrepA[hA].fi] -= 1;
				hA++;
			}
			
			while (hB != tB && strrepB[hB].se + 2 * WIN <= timestamp[pkt]) {
				strmapB[strrepB[hB].fi] -= 1;
				hB++;
			}

			strmapA[tuple] += 1;
			strrepA[tA++] = mp(tuple, timestamp[pkt]);


			sketch->insert((uint8_t *)(addr + pkt), timestamp[pkt]);

			if ((timestamp[pkt] - timestamp[0]) >= 2 * WIN
					&& (timestamp[pkt] / (WIN/100)) != (timestamp[pkt-1] / (WIN/100))) {
				vector<pii> hc1;
				for (auto &it: strmapA) {
					hc1.push_back(mp(it.fi, abs(strmapB[it.fi] - it.se)));
				}
				sort(All(hc1), cmp);
				hc1.resize(QK);
				vector<pii> hc2 = sketch->heavychange(QK, timestamp[pkt]);
				double same = 0, aee = 0, are = 0;
				rep2 (i, 0, SZ(hc1)) rep2 (j, 0, SZ(hc2)) {
					if (hc1[i].fi == hc2[j].fi) {
						same++;
						break;
					}
				}
				double recall = same / SZ(hc1);
				totrecall += recall;
				cnt++;
			}
			pkt++;
		}
		cout << totrecall / cnt << endl;
			
	
	}
    return 0;
}