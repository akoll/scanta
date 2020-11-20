from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun, PlotStub

runs = [
  Run(name='vot_par_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_VOT', steps=[Step(avg=True)]),
  Run(name='tov_par_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_TOV', steps=[Step(avg=True)]),
  Run(name='scattered_par_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_SCATTERED', steps=[Step(avg=True)]),
  Run(name='vot_seq_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL -DSTORAGE_VOT', steps=[Step(avg=True)]),
  Run(name='tov_seq_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL -DSTORAGE_TOV', steps=[Step(avg=True)]),
  Run(name='scattered_seq_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL -DSTORAGE_SCATTERED', steps=[Step(avg=True)]),

  Run(name='scattered_set_seq_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL -DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SET', steps=[Step(avg=True)]),
  Run(name='scattered_set_par_ft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SET', steps=[Step(avg=True)]),
]
  
benchmark = Benchmark(
  dir='fire_game/',
  main='../fire_game.cpp',
  frames=10000,
  compile_params='-lSDL2 -lSDL2_image',
  runs=runs
)

benchmark.generate()
