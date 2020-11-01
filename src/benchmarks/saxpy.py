from s2bench.cpp2bench import Benchmark

benchmark = Benchmark(
  dir='saxpy/',
  title='saxpy - parallel',
  xlabel='entity count',
  ylabel='frame time',
  main='../saxpy.cpp',
  frames=1000,
  instrument='native',
  compile_params='-DRUNTIME_PARALLEL -DINITIAL_COUNT=100000 -DSPAWN_RATE=1024 -DWIDTH=128',
  runs=[
    {
      'name': 'tuple of vectors',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_TOV',
      'tex_params': 'orange 1024 100000',
    },
    {
      'name': 'vector of tuples',
      'compile_params': '-DBENCHMARK_FRAMETIME -DSTORAGE_VOT',
      'tex_params': 'blue 1024 100000',
    },
  ]
)
