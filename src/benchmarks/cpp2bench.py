import os

class Benchmark:
  def __init__(self, title, xlabel, ylabel, main, frames, runs, instrument='native', compile_params='', run_params='', tex_params='', ymax=None, dir='build/'):
    self.dir = dir
    os.mkdir(dir)
    self.__graph(title, xlabel, ylabel, main, frames, runs, ymax)
    self.__makefile(main, frames, compile_params, run_params, tex_params, runs, instrument)

  def __graph(self, title, xlabel, ylabel, main, frames, runs, ymax=None):
    # print('graph:', title, xlabel, ylabel, main, frames, runs)
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
      .replace('%runs%', '\n'.join([self.__plot(run) for run in runs]))
    )
    file.close()

  def __plot(self, run):
    name = run['name']
    print('plot:', name)
    filename = name.replace(' ', '_') + '.tex'
    return r"""
      \input{%file%}
      \addlegendentry{%name%}
    """.rstrip().replace('%name%', name).replace('%file%', filename) + '\n'

  def __makefile(self, main, frames, compile_params, run_params, tex_params, runs, instrument):
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

    filenames = [run['name'].replace(' ', '_') for run in runs]
    for run, filename in zip(runs, filenames):
      name = run['name']
      compile_params = run['compile_params'] if 'compile_params' in run else ''
      tex_params = run['tex_params'] if 'tex_params' in run else ''
      run_instrument = run['instrument'] if 'instrument' in run else instrument
      file.write("""
%outfile%: %main% $(DEPS)
	$(CC) -MMD -o $@ $< $(CFLAGS) $(CINCLUDES) $(CPARAMS) %compile_params%
        """.strip()
        .replace('%main%', main)
        .replace('%compile_params%', compile_params)
        .replace('%outfile%', filename + '.out')
        + '\n\n'
      )
      if run_instrument == 'native':
        file.write("""
%texfile%: %outfile%
	%steps% | ../bench2tex.py %tex_params% > %texfile%
          """.strip()
          .replace('%tex_params%', tex_params)
          .replace('%outfile%', filename + '.out')
          .replace('%texfile%', filename + '.tex')
          .replace('%steps%', self._render_steps(run, filename))
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
          .replace('%texfile%', filename + '.tex')
          .strip()
          + '\n\n'
        )

    file.write("""
-include *.d

bench.pdf: graph.tex %texs%
	pdflatex --jobname=bench graph.tex

clean:
	rm -rf bench.pdf *.out *.d *.log *.aux %texs%

.PHONY: default run clean
      """.strip()
      .replace('%texs%', ' '.join([file + '.tex' for file in filenames]))
    )
      
    file.close()

  def _render_steps(self, run, filename):
    commands = '; '.join(['(./{} {}{})'.format(
      filename + '.out',
      step['params'],
      ' | ../bench2avg.py' if 'average' in step and step['average'] else '',
    ) for step in run['steps']])
    if (len(run['steps']) > 1):
      return '(' + commands + ')'
    else:
      return commands
