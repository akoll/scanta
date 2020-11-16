from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

step_calc = 'x+1'
ycalc = '"y"'

def step_count(x):
  return eval(step_calc)

steps = [
  Step(
    avg=True,
    #params=str(entities // (step_count(i))),
    params='100000',
    compile_params='-DSYSTEM_COUNT=' + str(step_count(i)),
  )
  for i in range(0, 8)
]

runs = [
  Run(
    name='tov',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
    steps=steps
  ),
  Run(
    name='vot',
    compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_VOT',
    steps=steps
  ),
  Run(
    name='tovc',
    compile_params='-DSTORAGE_TOV',
    steps=steps,
    instrument='perf',
  ),
  Run(
    name='votc',
    compile_params='-DSTORAGE_VOT',
    steps=steps,
    instrument='perf',
  ),
]

benchmark = Benchmark(
  dir='component_types/',
  #title='component types - sequential, $10^4$ frames, entities: $10^5$',
  title='',
  xlabel='number of component types',
  ylabel='average frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,ymin=0',
  ylabel_right='cache misses',
  axis_params_right='y unit=\\%,xmin=1,xmax=8,ymin=0,',
  main='../component_types.cpp',
  frames=10000,
  compile_params='-DSCHEDULER_SEQUENTIAL -DWIDTH=8u -DITERATIONS=32u',
  legend_shift='0.075',
  runs=runs,
  plots=[
    Plot('tovft', title='tuple of vectors', tex_params='"orange,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0])]),
    Plot('votft', title='vector of tuples', tex_params='"red,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1])]),
    Plot('tovcm', title='tuple of vectors', side='right', tex_params='"blue,thick,mark=*" "' + step_calc + '"', plotruns=[PlotRun(runs[2])]),
    Plot('votcm', title='vector of tuples', side='right', tex_params='"violet,thick,mark=*" "' + step_calc + '"', plotruns=[PlotRun(runs[3])]),
  ]
)

benchmark.generate()
