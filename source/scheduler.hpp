#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <ga/GASimpleGA.h>
#include <ga/GASStateGA.h>
#include <ga/GAGenome.h>
#include <ga/GARealGenome.h>

#include "model.hpp"
#include "schedule.hpp"

using namespace std;

namespace Fractal {
struct scheduler_config {
	int popsz;
	int ngen;
	float pmut;
	float pcross;
	float pconv;
};

struct user_data {
	Model *model;
	int slackjob;
	vector<Job> pinnedj;
	vector<Message> pinnedm;
	Schedule *base_schedule = nullptr;
	Horizon horizon_;

	user_data(Model *modelptr, Horizon h) :
			model(modelptr), horizon_(h) {
	}
};

class Scheduler {
public:
	Scheduler(Fractal::Model *model, Fractal::scheduler_config *config, Fractal::Horizon* horizon);

	~Scheduler();

	void evolve();
	Schedule best();

	//Fractal::Horizon getHorizon;

	Model model(){return *model_;}

private:
	Model *model_;
	GASteadyStateGA *ga_;
	user_data *data_;
	int popsz_;
	int ngen_;
	float pmut_;
	float pcross_;
	float pconv_;

	Fractal::Horizon horizon_;

	//void init(Fractal::Horizon* horizon);
	void init();

	static float objective(GAGenome &g);

	static Fractal::Schedule __genome_to_schedule(const GAGenome &g);
};
} // namespace Fractal
#endif
