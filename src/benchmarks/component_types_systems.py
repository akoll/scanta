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
    compile_params='-DBENCHMARK_FRAMETIME -DRUNTIME_SEQUENTIAL',
    steps=steps
  ),
  Run(
    name='par',
    compile_params='-DBENCHMARK_FRAMETIME -DRUNTIME_PARALLEL',
    steps=steps
  ),
]

benchmark = Benchmark(
  dir='component_types_systems/',
  #title='component types - sequential, $10^4$ frames, entities: $10^5$',
  title='',
  xlabel='number of registered systems',
  ylabel='average frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,',
  ylabel_right='compilation time',
  axis_params_right='y unit=s,',
  main='../component_types.cpp',
  frames=1000,
  compile_params='-DSTORAGE_TOV -DWIDTH=128u -DITERATIONS=128u -DUSE_ALL',
  legend_shift='0.075',
  runs=runs,
  plots=[
    Plot('seqft', title='sequential runtime', tex_params='"orange,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0])]),
    Plot('seqct', title='sequential runtime', side='right', tex_params='"orange!50!black,dashed,mark=x" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[0], cbench=True)]),
    Plot('parft', title='parallel runtime', tex_params='"red,thick,mark=*" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1])]),
    Plot('parct', title='parallel runtime', side='right', tex_params='"red!50!black,dashed,mark=x" "' + step_calc + '" ' + ycalc, plotruns=[PlotRun(runs[1], cbench=True)]),
  ]
)

benchmark.generate()
