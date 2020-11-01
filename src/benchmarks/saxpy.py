from s2bench.cpp2bench import Benchmark

initial = 32000
rate = 1024
steps = [{ 'params': str(initial + i * rate) } for i in range(10)]

benchmark = Benchmark(
  dir='saxpy/',
  title='saxpy - parallel',
  xlabel='entity count',
  ylabel='frame time',
  axis_params='change y base, y SI prefix=milli, y unit=s',
  main='../saxpy.cpp',
  frames=1000,
  instrument='native',
  compile_params='-DRUNTIME_PARALLEL -DSPAWN_RATE=1024 -DWIDTH=128',
  runs=[
    {
      'name': 'tuple of vectors',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
      'tex_params': 'orange,thick,mark=* {} {}'.format(rate, initial),
      'averages': True,
      'steps': steps,
    },
    {
      'name': 'vector of tuples',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_VOT',
      'tex_params': 'blue,thick,mark=* {} {}'.format(rate, initial),
      'averages': True,
      'steps': steps,
    },
  ]
)
