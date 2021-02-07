#include "../MicroSketch-CU.h"
const int M = 6e3, T = 1, H = 3, size_k = 4, log_base = 1;
const int TRACE_LEN = 16;
const int TUPLE_LEN = 4;
const int MAXINPUT = 1e7;
const int WIN = 1e6;
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
	srand(time(0));
	freopen("output.csv", "w", stdout);
	cout << "MicroSketch-CU" << endl;
	for (int mem = 100; mem <= 500; mem += 100) {
			strmap.clear();
			int M = mem * 1024 * 8 / H / (32-size_k + (T+2)*size_k + ceil(5 - log(log_base)/log(2)));
			auto sketch = new MicroSketch_CU<TUPLE_LEN>(M, WIN, T, size_k, H, log_base);
			double totaee = 0;
			int cnt = 0, pkt = 0, h = 0, t = 0;
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
					cnt++;
					int mapsize = 0;
					double sum = 0, sum_straw = 0, sum_new = 0, sum_neg = 0, sum_neg_new = 0, sum_cu = 0, sum_neg_cu = 0;
					for (auto &it: strmap) if (it.se) {
						mapsize++;
						sum += abs(it.se - sketch->query((uint8_t*)(&(it.fi)), timestamp[pkt]));
					}
					totaee += sum / mapsize;
				}
				pkt++;
			}
			cout << totaee / cnt << endl;
	}
	
    return 0;
}