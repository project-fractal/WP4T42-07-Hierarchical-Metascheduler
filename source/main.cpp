#include <iostream>
#include <sstream>
#include <string>

#include <bits/stdc++.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils/json.hpp"
#include "json_tools.hpp"
#include "model.hpp"
#include "metascheduler.hpp"
#include "context.hpp"

using json = nlohmann::json;
using namespace std;

int reconvHorizon;
int samp_freq;
int minx = 999;

int a,b, schedNum;
float c,d,e, average = 0;

Fractal::MsGraph msg;
Fractal::Calendar cal;

int main(int argc, char **argv) {
    // Creating a directory
    if (mkdir("results", 0777) == -1)
        cerr << "Error :  " << strerror(errno) << endl;

    else
        cout << "Directory created" << endl;

    cout << endl;

    string inputModel, contextModel, line;
    ifstream f("Parameters.txt");

	int line_num = 1;
	while (getline(f, line)) {
		stringstream g (line);

		if (line_num == 1)
			g >> inputModel;

		if (line_num == 2)
			g >> contextModel;

		if (line_num == 3) {
			g >> reconvHorizon;
		}
		if (line_num == 4) {
			g >> samp_freq;
		}
		if (line_num == 5) {
			g >> a;
		}
		if (line_num == 6) {
			g >> b;
		}
		if (line_num == 7) {
			g >> c;
		}
		if (line_num == 8) {
			g >> d;
		}
		if (line_num == 9) {
			g >> e;
		}
		if (line_num == 10) {
					g >> schedNum;
				}

		line_num++;
	}
		cout << inputModel << ", " << contextModel << ", " << reconvHorizon << ", " << samp_freq << endl;

	/* Variable to store application model*/
	json app;
	json ctx;
	app = ludwig::read_json(inputModel);
	ctx = ludwig::read_json(contextModel);

	Fractal::Model model(app);
	Fractal::Context context (ctx);
	float comp_time = 0;

	cout << "  Creating MsG  " << endl;
	for (int i=0;i<schedNum;i++) {
		minx = 999;
		auto t1 = high_resolution_clock::now();
		Fractal::createMSG(&model, &context);
		auto t2 = high_resolution_clock::now();
		auto ms_int = duration_cast<milliseconds>(t2 - t1);
		comp_time = comp_time + ms_int.count();
		cout << "Time to compute MSG: " << ms_int.count() << "ms\n" << endl;
	}
	cout << "Average Energy Saving: " << average/schedNum << endl;
	cout << "Average Computational Time: " << comp_time/schedNum << endl;

	return 0;
}
