import os

class Benchmark:
  def __init__(self, title, xlabel, ylabel, main, frames, executables, instrument='native', compile_params='', run_params='', tex_params='', ymax=None, dir='build/'):
    self.dir = dir
    os.mkdir(dir)
    self.__graph(title, xlabel, ylabel, main, frames, executables, ymax)
    self.__makefile(main, frames, compile_params, run_params, tex_params, executables, instrument)

  def __graph(self, title, xlabel, ylabel, main, frames, executables, ymax=None):
    # print('graph:', title, xlabel, ylabel, main, frames, runs)
    # runs = [run of executable in executables for run in executable['runs']]
    print('graph {}: {}'.format(main, title))
    file = open(self.dir + 'graph.tex', 'x')
    file.write(r"""
\documentclass{article}
\usepackage{graphics}
\usepackage{tikz}
\usepackage{pgfplots}

\begin{document}

\beginpgfgraphicnamed{bench}
  \begin{tikzpicture}
    \begin{axis}[
      title={\textbf{%title%}},
      width=12cm, height=8cm,
      axis lines=left,
      grid=major,
      xlabel={%xlabel%}, ylabel={%ylabel%},%ymax%
    ]
%runs%
    \end{axis}
  \end{tikzpicture}
\endpgfgraphicnamed

\end{document}
      """.strip()
      .replace('%title%', title)
      .replace('%xlabel%', xlabel)
      .replace('%ylabel%', ylabel)
      .replace('%ymax%', ' ymax={},'.format(ymax) if ymax else '')
      .replace('%runs%', '\n'.join([self.__plot(executable) for executable in executables]))
    )
    file.close()

  def __plot(self, executable):
    for index in range(len(executable['runs'])):
      name = executable['name']
      print('plot:', name)
      filename = name.replace(' ', '_') + '_' + str(index) + '.tex'
      return r"""
      \input{%file%}
      \addlegendentry{%name%}
      """.rstrip().replace('%name%', name).replace('%file%', filename) + '\n'

  def __makefile(self, main, frames, compile_params, run_params, tex_params, executables, instrument):
    file = open(self.dir + 'Makefile', 'x')
    file.write("""
CC = g++
CFLAGS = -Wall -std=c++2a -O3 -fopenmp
CINCLUDES = -I../.. -I../../../lib -I../../../lib/taskflow -I../../../lib/entt/src
CPARAMS = -DFRAME_COUNT=%frames% %compile_params%

default: bench.pdf

      """.strip()
      .replace('%frames%', str(frames))
      .replace('%compile_params%', compile_params)
      + '\n\n'
    )

    filenames = [executable['name'].replace(' ', '_') for executable in executables]
    for executable, filename in zip(executables, filenames):
      name = executable['name']
      compile_params = executable['compile_params'] if 'compile_params' in executable else ''
      tex_params = executable['tex_params'] if 'tex_params' in executable else ''
      file.write("""
%outfile%.out: %main% $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) $(CINCLUDES) $(CPARAMS) %compile_params%

%outfile%: %outfile%.out
	%runs%
	: > kek.lel
        """.strip()
        .replace('%main%', main)
        .replace('%compile_params%', compile_params)
        .replace('%outfile%', filename)
        .replace('%runs%', '\n'.join(['make ' + filename + '_' + str(index) + '.tex && \\' for index in range(len(executable['runs']))]))
        + '\n\n'
      )

      for index in range(len(executable['runs'])):
        run = executable['runs'][index]
        run_params = run['run_params'] if 'run_params' in run else ''
        run_instrument = run['instrument'] if 'instrument' in run else instrument
        if run_instrument == 'native':
          file.write("""
  %texfile%: %outfile%
    ./%outfile% %run_params% | ../bench2tex.py %tex_params% > %texfile%
            """.strip()
            .replace('%run_params%', run_params)
            .replace('%tex_params%', tex_params)
            .replace('%outfile%', filename + '.out')
            .replace('%texfile%', filename + '_' + str(index) + '.tex')
            + '\n\n'
          )
        elif run_instrument == 'perf':
          file.write("""
  %texfile%: %outfile%
    perf stat -e L1-dcache-loads,L1-dcache-load-misses -x , ./%outfile%
            """
            .replace('%run_params%', run_params)
            #.replace('%tex_params%', tex_params)
            .replace('%outfile%', filename + '.out')
            .replace('%texfile%', filename + '_' + str(index) + '.tex')
            .strip()
            + '\n\n'
          )

    file.write("""
-include *.d

bench.pdf: graph.tex %texs%
	pdflatex --jobname=bench graph.tex

clean:
	rm -rf *.out *.d *.log *.aux %texs%

.PHONY: default run clean
      """.strip()
      .replace('%texs%', ' '.join([filenames[exe_index] + '_' + str(run_index) + '.tex' for exe_index in range(len(executables)) for run_index in range(len(executables[exe_index]['runs']))]))
    )
      
    file.close()
