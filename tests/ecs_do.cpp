#include "ecs_do.hpp"
#include "common/log.hpp"
#include "common/time.hpp"

#include <iostream>

#include <stdio.h>
#include <unistd.h>

float appetites [] = {  20,  10,  10,  10,  10,  10 };
float preys     [] = {  10,  20,  10,  10,  10,  10 };
float libidos   [] = {   1,   1, 0.2,   1,   1,   1 };
float speeds    [] = {  20,  15,  20,  20,  15,  25 };

using test_func_type = void(*)();

// Function that runs the tests
test_func_type test_func;
// Number of entities to reach
size_t entity_count;
// Number of iterations between consecutive
// entity births
size_t increment_period;
// Accumulated times by number of entities
double * times;
// Variables for measuring time
time_point tp;

void run_measure_tick()
{
	fprintf(stderr, "# Tick test\n# Entity Count: %llu\n# Increment period: %llu\n",
		entity_count, increment_period);

	world w;

	double * times = new double[entity_count+1];
	for(size_t i = 0; i < entity_count * increment_period; ++i)
	{
		if(i % increment_period == 0)
		{
			uint64_t e = w.create();
			int s = i%6;
			w.spe.create(e, s);
			w.hea.create(e, appetites[s], (s+1)%6);
			w.mov.create(e, speeds[s]);
			w.pos.create(e, sin(i)*10, cos(i)*10);
			w.rep.create(e, libidos[s]);
			times[w.count()] = 0.0;
		}
		tp = now();
		w.update(1);
		times[w.count()] += elapsed(tp, now());

		fprintf(stderr, "\r%3d%%", 100*(i+1)/(entity_count*increment_period));
	}
	fprintf(stderr, "\n");

	for(int i = 0; i <= entity_count; ++i)
		std::cout << i << '\t' << times[i] / (double)increment_period << '\n';
	delete [] times;
}

void run_measure_insertion()
{
	fprintf(stderr, "# Insertion test\n# Entity count: %llu\n", entity_count);

	world w;

	double * times = new double[entity_count+1];
	for(size_t i = 0; i < entity_count; ++i)
	{
		int s = i%6;
		tp = now();

		uint64_t e = w.create();
		w.spe.create(e, s);
		w.hea.create(e, appetites[s], (s+1)%6);
		w.mov.create(e, speeds[s]);
		w.pos.create(e, sin(i)*10, cos(i)*10);
		w.rep.create(e, libidos[s]);

		times[w.count()] = elapsed(tp, now());

		fprintf(stderr, "\r%3d%%", 100*(i+1)/entity_count);
	}
	fprintf(stderr, "\n");

	for(int i = 0; i <= entity_count; ++i)
		std::cout << i << '\t' << times[i] << '\n';
	delete [] times;
}

void run_measure_fps()
{
	fprintf(stderr, "# FPS test\n");

	world w;
	double accum_time;
	size_t count = 0;

	while(true)
	{
		// Create new entity
		uint64_t e = w.create();
		int s = count%6;
		w.spe.create(e, s);
		w.hea.create(e, appetites[s], (s+1)%6);
		w.mov.create(e, speeds[s]);
		w.pos.create(e, sin(count)*10, cos(count)*10);
		w.rep.create(e, libidos[s]);
		++count;
		// Reset time count
		accum_time = 0.0;

		for(size_t i = 0; i < 10; ++i)
		{
			tp = now();
			w.update(1);
			accum_time += elapsed(tp, now());
		}
		double avg_time = accum_time / 10.0;
		fprintf(stderr, "\r%-6.2f%%", 6000.0f*avg_time);

		if(avg_time > 1.0/60.0) {
			fprintf(stderr, "\r100%%   \n");
			printf("%zu\n", count-1);
			return;
		}
	}
}

int main(int argc, char ** argv)
{
	enum { TICK, INSERTION, FPS } test;
	test_func = run_measure_tick;
	test = TICK;
	int c;
	while((c = getopt(argc, argv, "tif")) != -1)
		switch(c)
		{
			case 't':
				test_func = run_measure_tick;
				test = TICK;
				break;
			case 'i':
				test_func = run_measure_insertion;
				test = INSERTION;
				break;
			case 'f':
				test_func = run_measure_fps;
				test = FPS;
				break;
			case '?':
				fprintf(stderr, "unknown option '-%c'\n", optopt);
			default:
				return -1;
		}

	switch(test)
	{
		case TICK:
			if(optind != argc-2)
				fatal("tick test requires two arguments: entity count and increment period");
			entity_count = atoll(argv[optind]);
			increment_period = atoll(argv[optind+1]);
			break;
		case INSERTION:
			if(optind != argc-1)
				fatal("insertion test requires one argument: entity count");
			entity_count = atoll(argv[optind]);
			break;
		case FPS:
			break;
			// if(optind != argc-1)
			// 	fatal("FPS test requires one argument: increment period");
			// increment_period = atoll(argv[optind]);
	}

	test_func();

	return 0;
}
