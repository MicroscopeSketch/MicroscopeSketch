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
	freopen("speed.csv", "w", stdout);
	cout << "MicroSketch-CU" << endl;
	for (int mem = 100; mem <= 500; mem += 100) {
		double mops = 0;
		for (int test = 1; test <= 40; test++) {
			strmap.clear();
			int M = mem * 1024 * 8 / H / (32-size_k + (T+2)*size_k + ceil(5 - log(log_base)/log(2)));
			auto sketch = new MicroSketch_CU<TUPLE_LEN>(M, WIN, T, size_k, H, log_base);
			double totaee = 0;
			int cnt = 0, pkt = 0, h = 0, t = 0;

			clock_t st = clock();

			while (pkt < NUM * WIN) {
				sketch->insert((uint8_t *)(addr + pkt), timestamp[pkt]);
				pkt++;
			}

			clock_t ed = clock();

			mops += 1. / (1. * (ed - st) / CLOCKS_PER_SEC / (NUM * WIN)) / 1e6;
		}
		cout << mops / 40 << endl;
			
	}
	
    return 0;
}