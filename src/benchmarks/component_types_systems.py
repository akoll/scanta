from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

step_calc = 'x+1'
ycalc = '"y"'

def step_count(x):
  return eval(step_calc)

steps = [
  Step(
    avg=True,
    #params=str(entities // (step_count(i))),
    params='128',
    compile_params='-DSYSTEM_COUNT=' + str(step_count(i)),
  )
  for i in range(0, 12)
]

runs = [
  Run(
    name='seq',
    compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_SEQUENTIAL',
    steps=steps,
    measure_compile_time=True
  ),
  Run(
    name='par',
    compile_params='-DBENCHMARK_FRAMETIME -DSCHEDULER_PARALLEL',
    steps=steps,
    measure_compile_time=True
  ),
]

benchmark = Benchmark(
  dir='component_types_systems/',
  #title='component types - sequential, $10^4$ frames, entities: $10^5$',
  title='',
  xlabel='number of registered systems',
  ylabel='average frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,xmin=1,xmax=12,ymin=0',
  ylabel_right='compilation time',
  axis_params_right='y unit=s,xmin=1,xmax=12,ymin=2,ymax=12.5',
  main='../component_types.cpp',
  frames=1000,
  compile_params='-DSTORAGE_TOV -DWIDTH=128u -DITERATIONS=128u -DUSE_ALL',
  legend_shift='0.075',
  arrowheads=False,
  runs=runs,
  plots=[
    Plot('seqft', title='sequential scheduler', tex_params='"red,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0])]),
    Plot('seqct', title='sequential scheduler', side='right', tex_params='"red,dashed,mark=x" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0], cbench=True)]),
    Plot('parft', title='parallel scheduler', tex_params='"blue,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1])]),
    Plot('parct', title='parallel scheduler', side='right', tex_params='"blue,dashed,mark=x" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1], cbench=True)]),
  ]
)

benchmark.generate()
