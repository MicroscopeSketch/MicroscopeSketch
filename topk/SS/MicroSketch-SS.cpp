#include "../MicroSketch-SS.h"
const int M = 6e3, T = 4, D = 8, size_k = 6, log_base = 1;
const int QK = 500;
const int TRACE_LEN = 16;
const int TUPLE_LEN = 4;
const int WIN = 1e6;
const int MAXINPUT = 1e7;
const int NUM = 3, LIM = 500;
uint32_t addr[MAXINPUT];
uint64_t timestamp[MAXINPUT];
pil strrep[MAXINPUT];
map<uint32_t, int> strmap;

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
	cout << "MicroSketch-SS" << endl;
	for (int mem = 80; mem <= 640; mem <<= 1) {
		strmap.clear();
		int M = mem * 1024 * 8 / (32 + (T+2)*size_k + 32-size_k + ceil(5 - log(log_base)/log(2)));
		auto sketch = new MicroSketch_SS<TUPLE_LEN>(M, WIN, T, D, size_k, log_base);
		double totare = 0, totrecall = 0;
		int cnt = 0, h = 0, t = 0, pkt = 0;
		while (cnt < 100 * (NUM-1)) {
			uint32_t tuple = addr[pkt];

			while (h != t && strrep[h].se + WIN <= timestamp[pkt]) {
				strmap[strrep[h].fi] -= 1;
				h++;
			}
			strmap[tuple] += 1;
			strrep[t++] = mp(tuple, timestamp[pkt]);


			sketch->insert((uint8_t *)(addr + pkt), timestamp[pkt]);

			if ((timestamp[pkt] - timestamp[0]) >= WIN
					&& (timestamp[pkt] / (WIN/100)) != (timestamp[pkt-1] / (WIN/100))) {
				vector<pii> topk1;
				for (auto &it: strmap) {
					topk1.push_back(it);
				}
				sort(All(topk1), cmp);
				vector<pii> topk2 = sketch->topk(QK, timestamp[pkt]);
				double same = 0, aee = 0, are = 0;
				rep2 (i, 0, QK) rep2 (j, 0, SZ(topk2)) {
					if (topk1[i].fi == topk2[j].fi) {
						same++;
						aee += abs(topk1[i].se - topk2[j].se);
						are += 1.0 * abs(topk1[i].se - topk2[j].se) / topk1[i].se;
						break;
					}
				}
				aee /= same; are /= same;
				double recall = same / QK;
				totare += are;
				totrecall += recall;
				cnt++;
			}
			pkt++;
		}
		cout << totrecall / cnt << "," << totare / cnt << endl;	
	}
    return 0;
}