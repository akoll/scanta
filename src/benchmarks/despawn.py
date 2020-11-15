from s2bench.cpp2bench import Benchmark, Run, Step, Plot, PlotRun

runs = [
  Run(
    name='ft',
    compile_params='',
    instrument='frameavg',
    repetitions=4,
    steps=[
      Step(compile_params='-DSTORAGE_SCATTERED'),
      Step(compile_params='-DSTORAGE_SCATTERED -DSCATTERED_SET'),
      Step(compile_params='-DSTORAGE_SCATTERED -DREVERSE'),
      #Step(compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_TOV'),
      #Step(compile_params='-DBENCHMARK_FRAMETIME -DSTORAGE_TOV -DREVERSE'),
    ]
  ),
  # Run(
  #   name='vecrft',
  #   compile_params='-DBENCHMARK_FRAMETIME -DREVERSE',
  #   instrument='frameavg',
  #   repetitions=4,
  # ),
  # Run(
  #   name='setft',
  #   compile_params='-DBENCHMARK_FRAMETIME -DSCATTERED_SET',
  #   instrument='frameavg',
  #   repetitions=4,
  # ),
]

benchmark = Benchmark(
  dir='despawn/',
  #title='spawn - sequential, $2500$ frames, spawn rate: $256$',
  title='',
  xlabel='',
  ylabel='time',
  width=8,
  height=8,
  axis_params='y unit=s,ybar,bar width=1cm,ymin=0,xtick=data,xticklabels={vector - from front, set, vector - from back},xmin=-0.5,xmax=2.5',
  main='../despawn.cpp',
  frames=1,
  compile_params='-DRUNTIME_SEQUENTIAL -DINITIAL_COUNT=1000000 -DDESPAWN_RATE=10000',
  runs=runs,
  plots=[
    Plot('ft', title='average frame time', tex_params='"fill=brown"', plotruns=[PlotRun(runs[0])], legend_entry=False),
    # Plot('vecft', title='vector - first entities', tex_params='"thick,brown" "x*256"', plotruns=[PlotRun(runs[0])]),
    # Plot('vecrft', title='vector - last entities', tex_params='"thick,red" "x*256"', plotruns=[PlotRun(runs[1])]),
    # Plot('setft', title='set', tex_params='"thick,blue" "x*256"', plotruns=[PlotRun(runs[2])]),
  ]
)

benchmark.generate()
