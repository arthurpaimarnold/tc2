#include "ecs_oo.hpp"
#include "common/log.hpp"
#include "common/time.hpp"
#include "common/util.hpp"

// #include <unistd.h>
// #include <stdlib.h>
#include <iostream>
#include <map>

#include <cstring>
#include <cassert>
#include <stdio.h>
#include <unistd.h>

struct run {
	int index;
	size_t entities;
	double accum_time;
	size_t remaining_passes;
};

void populate(world & w, size_t count)
{
	for(size_t i = 0; i < count; ++i)
	{
		auto & e = w.create();
		int s = irand(0, 7);
		e.add_component<species_component>(s);
		e.add_component<position_component>(drand(0, 100), drand(0, 100));

		if(s != 1 && s != 4)
		{
			e.add_component<health_component>(drand(1, 4), 1 + irand(0,1)*3);
			e.add_component<movement_component>(drand(10, 18));
			e.add_component<reproduction_component>(drand(1,4));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void run_measure_time(size_t entities, size_t increment, size_t passes, size_t iterations)
{
	fprintf(stderr, "# Time test\n# Entities: %zu\n# Increment: %zu\n# Passes: %zu\n# Iterations: %zu\n",
		entities, increment, passes, iterations);

	size_t run_count = entities/increment;
	size_t total_passes = run_count * passes;
	size_t passes_completed = 0;

	time_point tp;
	double * times = new double[run_count];

	std::vector<run> runs { run_count, { 0, 0, 0.0, passes } };
	for(size_t i = 0; i < run_count; ++i) {
		runs[i].index = i;
		runs[i].entities = (i+1) * increment;
	}

	while(!runs.empty())
	{
		int index = random() % runs.size();
		run & this_run = runs[index];

		world w;

		// Populate world
		populate(w, this_run.entities);
		assert(w.get_entity_count() == this_run.entities);

		fprintf(stderr, "\r%6.2f%%", 100.0f*(float)passes_completed/(float)total_passes);

		// Run iterations
		for(size_t i = 0; i < iterations; ++i)
		{
			tp = now();
			w.update(1);
			this_run.accum_time += elapsed(tp, now());
		}

		if(--this_run.remaining_passes == 0)
		{
			times[this_run.index] = this_run.accum_time / (double)(iterations*passes);
			runs.erase(runs.begin()+index);
		}
		++passes_completed;
	}
	fprintf(stderr, "\r   100%%\n");

	for(size_t i = 0; i < run_count; ++i)
		printf("%llu\t%f\n", (i+1)*increment, times[i]);

	delete [] times;
}

////////////////////////////////////////////////////////////////////////////////

void run_measure_insertion(size_t entities, size_t increment)
{
	fprintf(stderr, "# Insertion test\n# Entities: %zu\n# Increments: %zu\n", entities, increment);

	world w;

	time_point tp;
	size_t measures = entities/increment;
	double * times = new double[measures];

	tp = now();
	for(size_t m = 0; m < measures; ++m)
	{
		for(size_t i = 0; i < increment; ++i)
		{
			auto & e = w.create();
			e.add_component<species_component>(0);
			e.add_component<position_component>(0,0);
			e.add_component<health_component>(0,0);
			e.add_component<movement_component>(0);
			e.add_component<reproduction_component>(0);
		}
		times[m] = elapsed(tp, now());
	}
	for(size_t m = 0; m < measures; ++m)
		printf("%zu\t%f\n", (m+1)*increment, times[m]);
}

////////////////////////////////////////////////////////////////////////////////

void run_measure_fps(size_t iterations)
{
  fprintf(stderr, "# FPS test\n# Iterations: %zu\n", iterations);

  size_t lo = 0;
  size_t hi = 10000;
  bool hi_found = false;

  size_t cur;
  double t;
  time_point tp;
  while(true)
  {
    cur = (hi + lo) / 2;
    fprintf(stderr, "%zu ", cur);
    world w;
    populate(w, cur);
    t = 0;
    for(int i = 0; i < iterations; ++i)
    {
      tp = now();
      w.update(1);
      t += elapsed(tp, now());
    }
    t /= (double)iterations;
    fprintf(stderr, "%f\n", t);
    if(t > 1.0/60.0) {
      hi = cur;
      hi_found = true;
    } else {
      lo = cur;
      if(!hi_found)
        hi *= 2;
    }
    if(hi - lo <= 1) {
      printf("%zu %f\n", hi < 1.0/60.0 ? hi : lo, t);
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
	srandom(time(NULL));

	enum { TICK, INSERTION, FPS } test = TICK;

	int c;
	while((c = getopt(argc, argv, "tif")) != -1)
		switch(c)
		{
			case 't':
				test = TICK;
				break;
			case 'i':
				test = INSERTION;
				break;
			case 'f':
				test = FPS;
				break;
			case '?':
				fprintf(stderr, "unknown option '-%c'\n", optopt);
			default:
				return -1;
		}

	size_t entities;
	size_t increment;
	size_t iterations;
	size_t passes;

	switch(test)
	{
		case TICK:
			if(optind != argc-4)
				fatal("tick test requires four arguments: <entities> <increment> <passes> <iterations>");
			entities = atoll(argv[optind++]);
			increment = atoll(argv[optind++]);
			passes = atoll(argv[optind++]);
			iterations = atoll(argv[optind++]);
			run_measure_time(entities, increment, passes, iterations);
			break;
		case INSERTION:
			if(optind != argc-2)
				fatal("insertion test requires two arguments: <entities> <increment>");
			entities = atoll(argv[optind++]);
			increment = atoll(argv[optind++]);
			run_measure_insertion(entities, increment);
			break;
		case FPS:
			if(optind != argc-1)
				fatal("FPS test requires one argument: <iterations>");
			iterations = atoll(argv[optind++]);
      run_measure_fps(iterations);
			break;
	}

	return 0;
}
