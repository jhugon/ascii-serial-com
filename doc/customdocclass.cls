% 2020/05/31 Justin Hugon 

% --- Class structure: identification part
% ---
\ProvidesClass{customdocclass}[2020/05/31 version 1.0 Custom class for data format and API documentation]
\NeedsTeXFormat{LaTeX2e}

\LoadClass[letterpaper,12pt]{article}

\RequirePackage{expl3}
%\RequirePackage[letterpaper,hmargin=1in,vmargin=1in]{geometry}

\RequirePackage[utf8]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage{textcomp}
\RequirePackage[english]{babel}
\RequirePackage{amsmath}
\RequirePackage{amssymb}
\RequirePackage{bm}
\RequirePackage{mathpazo} % palatino like font for mathand text
\RequirePackage{inconsolata} % fixed width font
\RequirePackage{microtype}
\RequirePackage{lastpage}
\RequirePackage{titling}
\RequirePackage{xparse}
\RequirePackage{bm}
\RequirePackage[table]{xcolor}
\RequirePackage{siunitx}
\RequirePackage{isotope}
\RequirePackage{graphicx}
\RequirePackage{array}
\RequirePackage{tabularx}
\RequirePackage{tabulary}
\RequirePackage{multirow}
\RequirePackage{booktabs}
\RequirePackage{setspace}
\RequirePackage{tikz}
\usetikzlibrary{calc}
\usetikzlibrary{datavisualization}
\usetikzlibrary{datavisualization.formats.functions}
\usetikzlibrary{quotes}
%\usetikzlibrary{backgrounds}
%\usetikzlibrary{fit}
\usetikzlibrary{arrows}
\tikzset{>=latex}
\RequirePackage{pgfplots}
\RequirePackage{wrapfig}
\RequirePackage{hyperref}
\RequirePackage{cleveref}
\RequirePackage{listings}

\definecolor{mygreen}{rgb}{0,0.6,0}
\definecolor{mygray}{rgb}{0.5,0.5,0.5}
\definecolor{mymauve}{rgb}{0.58,0,0.82}

\lstset{ 
  backgroundcolor=\color{white},   % choose the background color; you must add \usepackage{color} or \usepackage{xcolor}; should come as last argument
  basicstyle=\footnotesize\tt,        % the size of the fonts that are used for the code
  breakatwhitespace=false,         % sets if automatic breaks should only happen at whitespace
  breaklines=true,                 % sets automatic line breaking
  captionpos=b,                    % sets the caption-position to bottom
  commentstyle=\color{mygreen},    % comment style
  deletekeywords={...},            % if you want to delete keywords from the given language
  escapeinside={\%*}{*)},          % if you want to add LaTeX within your code
  extendedchars=true,              % lets you use non-ASCII characters; for 8-bits encodings only, does not work with UTF-8
  %firstnumber=1000,                % start line enumeration with line 1000
  frame=single,	                   % adds a frame around the code
  keepspaces=true,                 % keeps spaces in text, useful for keeping indentation of code (possibly needs columns=flexible)
  keywordstyle=\color{blue},       % keyword style
  language=Octave,                 % the language of the code
  morekeywords={*,...},            % if you want to add more keywords to the set
  numbers=left,                    % where to put the line-numbers; possible values are (none, left, right)
  numbersep=5pt,                   % how far the line-numbers are from the code
  numberstyle=\tiny\color{mygray}, % the style that is used for the line-numbers
  rulecolor=\color{black},         % if not set, the frame-color may be changed on line-breaks within not-black text (e.g. comments (green here))
  showspaces=false,                % show spaces everywhere adding particular underscores; it overrides 'showstringspaces'
  showstringspaces=false,          % underline spaces within strings only
  showtabs=false,                  % show tabs within strings adding particular underscores
  %stepnumber=2,                    % the step between two line-numbers. If it's 1, each line will be numbered
  stringstyle=\color{mymauve},     % string literal style
  tabsize=2,	                   % sets default tabsize to 2 spaces
  title=\lstname                   % show the filename of files included with \lstinputlisting; also try caption instead of title
}

\newcolumntype{W}{>{\raggedleft\arraybackslash}X} % ragged left fill in width in tabularx
\newcolumntype{Y}{>{\raggedright\arraybackslash}X} % ragged right fill in width in tabularx

%Prefer PDF over png/jpeg
\DeclareGraphicsExtensions{.pdf,.png,.gif,.jpg}

\sisetup{per-mode=symbol}

\definecolor{darkgreen}{HTML}{008000}
\definecolor{light-gray}{gray}{0.8}
\DeclareSIUnit\lightyear{ly}
\DeclareSIUnit\parsec{pc}

\newcommand{\jra}{\ensuremath{\rightarrow}}
\newcommand{\tred}[1]{\textcolor{red}{#1}}
\newcommand{\tblue}[1]{\textcolor{blue}{#1}}
\newcommand{\tgreen}[1]{\textcolor{darkgreen}{#1}}
\newcommand{\tredbf}[1]{\textcolor{red}{\bf #1}}
\newcommand{\tbluebf}[1]{\textcolor{blue}{\bf #1}}
\newcommand{\tgreenbf}[1]{\textcolor{darkgreen}{\bf #1}}
\newcommand{\keV}{\ensuremath{\,\mathrm{ke\kern -0.1em V}}}
\newcommand{\keVc}{\ensuremath{\,\mathrm{ke\kern -0.1em V\!/c}}}
\newcommand{\keVcc}{\ensuremath{\,\mathrm{ke\kern -0.1em V\!/c}^2}}
\newcommand{\MeV}{\ensuremath{\,\mathrm{Me\kern -0.1em V}}}
\newcommand{\MeVc}{\ensuremath{\,\mathrm{Me\kern -0.1em V\!/c}}}
\newcommand{\MeVcc}{\ensuremath{\,\mathrm{Me\kern -0.1em V\!/c}^2}}
\newcommand{\GeV}{\ensuremath{\,\mathrm{Ge\kern -0.1em V}}}
\newcommand{\GeVc}{\ensuremath{\,\mathrm{Ge\kern -0.1em V\!/c}}}
\newcommand{\GeVcc}{\ensuremath{\,\mathrm{Ge\kern -0.1em V\!/c}^2}}
\newcommand{\TeV}{\ensuremath{\,\mathrm{Te\kern -0.1em V}}}
\newcommand{\TeVc}{\ensuremath{\,\mathrm{Te\kern -0.1em V\!/c}}}
\newcommand{\TeVcc}{\ensuremath{\,\mathrm{Te\kern -0.1em V\!/c}^2}}
\newcommand{\ihat}{\ensuremath{\,\bm{\hat{\imath}}}}
\newcommand{\jhat}{\ensuremath{\,\bm{\hat{\jmath}}}}
\newcommand{\khat}{\ensuremath{\,\bm{\hat{k}}}}
