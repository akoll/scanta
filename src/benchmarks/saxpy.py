from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

scattered_smart = Run(
  name='scattered - smart pointers',
  compile_params='-DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SMART',
  run_params='1000000'
)

scattered = Run(
  name='scattered',
  compile_params='-DSTORAGE_SCATTERED',
  run_params='100000'
)

vot = Run(
  name='vector of tuples',
  compile_params='-DSTORAGE_VOT',
  run_params='100000'
)

tov = Run(
  name='tuple of vectors',
  compile_params='-DSTORAGE_TOV',
  run_params='100000'
)

runs = [tov, scattered, vot]

width = 64

benchmark = Benchmark(
  dir='saxpy/',
  title='saxpy - sequential, $10^3$ frames, width: ${}$, entities: $10^5$'.format(width),
  xlabel='',
  ylabel='frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0,ybar,xtick=data,xticklabels={},grid=both,minor tick num=3'.format(
    '{' + ', '.join([run.name for run in runs]) + '}'
  ),
  arrowheads=False,
  main='../saxpy.cpp',
  frames=1000,
  compile_params='-DRUNTIME_SEQUENTIAL -DWIDTH={} -DBENCHMARK_FRAMETIME'.format(width),
  runs=runs,
  plots=[
    Plot('minimum frame time', tex_params='"fill=brown!50"', plotruns=[PlotRun(run, min=True) for run in runs]),
    Plot('average frame time', tex_params='"fill=brown!75"', plotruns=[PlotRun(run, avg=True) for run in runs]),
    Plot('maximum frame time', tex_params='"fill=brown"', plotruns=[PlotRun(run, max=True) for run in runs]),
  ]
)

benchmark.generate()
