from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

step_calc = '(x+1)*16'
ycalc = '"y"'

def step_count(x):
  return eval(step_calc)

steps = [
  Step(
    avg=True,
    #params=str(entities // (step_count(i))),
    #params=str(step_count(i)),
    params='4',
    compile_params='-DWIDTH={}u'.format(
      str(step_count(i)),
      str(step_count(i))
    ),
  )
  for i in range(0, 8)
]

runs = [
  Run(
    name='seq',
    compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL',
    steps=steps
  ),
  Run(
    name='par',
    compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL',
    steps=steps
  ),
]

benchmark = Benchmark(
  dir='component_types_small_systems/',
  #title='component types - sequential, $10^4$ frames, entities: $10^5$',
  title='',
  xlabel='width',
  ylabel='average frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,xmin=16,xmax=128,ymin=0,',
  main='../component_types.cpp',
  frames=10000,
  compile_params='-DSTORAGE_TOV -DSYSTEM_COUNT=8 -DITERATIONS=32u -DUSE_ALL',
  legend_shift='0.075',
  runs=runs,
  plots=[
    Plot('seqft', title='sequential scheduler', tex_params='"red,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0])]),
    Plot('parft', title='parallel scheduler', tex_params='"blue,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1])]),
  ]
)

benchmark.generate()
