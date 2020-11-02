from s2bench.cpp2bench import Benchmark

initial = 2**21
steps = [{ 'params': str(initial // (2*(2**i))), 'compile_params': '-DWIDTH=' + str(2**i) } for i in range(4, 16)]

benchmark = Benchmark(
  dir='saxpy/',
  title='saxpy',
  xlabel='vector width',
  ylabel='frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s,xmode=log,log basis x=2',
  main='../saxpy.cpp',
  frames=10000,
  instrument='native',
  compile_params='-DRUNTIME_SEQUENTIAL',
  runs=[
    {
      'name': 'tuple of vectors',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
      'tex_params': 'orange,thick,mark=* \'2**(4+x)\'',
      'averages': True,
      'steps': steps,
    },
    {
      'name': 'vector of tuples',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_VOT',
      'tex_params': 'blue,thick,mark=* \'2**(4+x)\'',
      'averages': True,
      'steps': steps,
    },
  ]
)
