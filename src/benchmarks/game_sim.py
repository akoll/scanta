from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun, PlotStub

runs = [
  Run(name='votft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_VOT'),
  Run(name='tovft', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_TOV'),
  Run(name='scattered', compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL -DSTORAGE_SCATTERED'),
]
  
benchmark = Benchmark(
  dir='game_sim/',
  main='../game_sim.cpp',
  frames=50000,
  title='',
  arrowheads=False,
  ylabel='frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0,grid=both,minor tick num=3',
  compile_params='-lSDL2 -lSDL2_image',
  runs=runs,
  plots=[
    Plot('votft', title='vector of tuples', tex_params='thin,red', plotruns=[PlotRun(runs[0])]),
    Plot('tovft', title='tuple of vectors', tex_params='thin,orange', plotruns=[PlotRun(runs[1])]),
    Plot('scatteredft', title='tuple of vectors', tex_params='thin,blue', plotruns=[PlotRun(runs[2])]),
  ]
)

benchmark.generate()
