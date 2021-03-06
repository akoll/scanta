from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun, PlotStub

scattered_smart = Run(
  name='shared pointers, vector',
  compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SMART',
  run_params='1000000'
)

scattered = Run(
  name='raw pointers, vector',
  compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED',
  run_params='100000'
)

scattered_smart_set = Run(
  name='shared pointers, set',
  compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SMART -DSTORAGE_SCATTERED_SET',
  run_params='1000000'
)

scattered_set = Run(
  name='raw pointers, set',
  compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SET',
  run_params='100000'
)

cm = Run(
  name='cm',
  run_params='1000000',
  instrument='perf',
  steps=[
    Step(compile_params='-DSTORAGE_SCATTERED -DSTORAGE_SMART'),
    Step(compile_params='-DSTORAGE_SCATTERED'),
    Step(compile_params='-DSTORAGE_SCATTERED -DSTORAGE_SMART -DSTORAGE_SCATTERED_SET'),
    Step(compile_params='-DSTORAGE_SCATTERED -DSTORAGE_SCATTERED_SET'),
  ]
)

runs = [scattered_smart, scattered_smart_set, scattered, scattered_set]

width = 64

benchmark = Benchmark(
  dir='saxpy_smart/',
  title='',
  width=10,
  height=8,
  xlabel='',
  ylabel='frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0,ybar,xtick=data,xticklabels={},xmin=-0.5,xmax=3.5,grid=both,minor tick num=3,'.format(
    '{' + ', '.join(['{' + run.name + '}' for run in runs]) + '}'
  ) + 'xticklabel style={rotate=-45,yshift=4mm,xshift=2mm,align=left}',
  ylabel_right='cache misses',
  axis_params_right='ybar, y unit=percent,ymin=0,xmin=-0.5,xmax=3.5',
  arrowheads=False,
  main='../saxpy.cpp',
  frames=1000,
  compile_params='-DSCHEDULER_SEQUENTIAL -DWIDTH={}'.format(width),
  runs=[*runs, cm],
  plots=[
    Plot('minimum frame time', tex_params='"fill=brown!50"', plotruns=[PlotRun(run, min=True) for run in runs]),
    Plot('average frame time', tex_params='"fill=brown!75"', plotruns=[PlotRun(run, avg=True) for run in runs]),
    Plot('maximum frame time', tex_params='"fill=brown"', plotruns=[PlotRun(run, max=True) for run in runs]),
    PlotStub(side='left'),
    PlotStub(side='right'),
    PlotStub(side='right'),
    PlotStub(side='right'),
    Plot('cache misses', tex_params='"fill=blue!80!black"', side='right', plotruns=[PlotRun(cm)]),
  ]
)

benchmark.generate()
