\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{qrsxtjyyccp}[2026/10/18 QRSXTJYY course-design report]

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{cqutreport}}
\ProcessOptions\relax
\LoadClass{cqutreport}

\RequirePackage[
  top=2.5cm,
  right=2.5cm,
  bottom=2.5cm,
  left=3cm,
  headheight=15pt,
  headsep=0.45cm,
  footskip=0.72cm
]{geometry}
\RequirePackage{xcolor}
\RequirePackage{array}
\RequirePackage{tabularx}
\RequirePackage{makecell}
\RequirePackage{multirow}
\RequirePackage{enumitem}
\RequirePackage{fancyhdr}
\RequirePackage{listings}
\RequirePackage{hyperref}

\setmainfont{Times New Roman}
\setsansfont{Arial}
\setmonofont{Menlo}

\hypersetup{hidelinks}
\renewcommand{\arraystretch}{1.0}
\definecolor{deepblue}{RGB}{0,0,128}

\newcolumntype{C}[1]{>{\centering\arraybackslash}p{#1}}
\newcolumntype{L}[1]{>{\raggedright\arraybackslash}p{#1}}
\newcommand{\rednote}[1]{{\color{red}#1}}
\newcommand{\bluenote}[1]{{\color{blue}#1}}
\newcommand{\deepbluenote}[1]{{\color{deepblue}#1}}
\newcommand{\notepara}[2]{%
  {\setlength{\leftskip}{2em}\setlength{\parindent}{0pt}\color{#1}#2\par}%
}
\newcommand{\bluepara}[1]{\notepara{blue}{#1}}
\newcommand{\redpara}[1]{\notepara{red}{#1}}
\newcommand{\deepbluepara}[1]{\notepara{deepblue}{#1}}
\newcommand{\blankline}[1]{\makebox[#1]{\hrulefill}}
\newcommand{\coverrow}[4]{%
  \noindent\makebox[2.0cm][s]{#1}\underline{\makebox[#2][c]{#3}}\makebox[#4]{}\\[0.42cm]%
}

\fancypagestyle{reporthead}{
  \fancyhf{}
  \fancyhead[L]{\zihao{-5}重庆理工大学综合课程设计报告}
  \fancyhead[R]{\zihao{-5}物联网工程专业-综合课程设计I-桌面用电监控}
  \renewcommand{\headrulewidth}{0.4pt}
  \renewcommand{\footrulewidth}{0pt}
}

\fancypagestyle{reportbody}{
  \fancyhf{}
  \fancyhead[L]{\zihao{-5}重庆理工大学综合课程设计报告}
  \fancyhead[R]{\zihao{-5}物联网工程专业-综合课程设计I-桌面用电监控}
  \fancyfoot[C]{\zihao{-5}\thepage}
  \renewcommand{\headrulewidth}{0.4pt}
  \renewcommand{\footrulewidth}{0pt}
}

\lstdefinestyle{appendixcode}{
  basicstyle=\ttfamily\zihao{5},
  breaklines=true,
  columns=fullflexible,
  keepspaces=true,
  showstringspaces=false,
  tabsize=4,
  xleftmargin=2em,
  aboveskip=0.5em,
  belowskip=0.5em
}

\newcommand{\tocline}[2]{%
  \noindent\makebox[\textwidth][l]{\heiti\bfseries #1\dotfill #2}\par\vspace{0.05cm}%
}
\newcommand{\tocsubline}[2]{%
  \noindent\hspace{0.72cm}\makebox[\dimexpr\textwidth-0.72cm\relax][l]{\heiti\bfseries #1\dotfill #2}\par\vspace{0.05cm}%
}
\newcommand{\sectiontitle}[2]{%
  \vspace*{-0.32cm}%
  {\noindent\heiti\bfseries\fontsize{18pt}{20pt}\selectfont #1.\quad #2\par}
  \vspace{0.50cm}
}
\newcommand{\subsectiontitle}[2]{%
  \vspace{0.34cm}
  {\noindent\heiti\bfseries\fontsize{16pt}{18pt}\selectfont #1.\quad #2\par}
  \vspace{0.54cm}
}
\newcommand{\itemline}[2]{%
  {\setlength{\leftskip}{2em}\setlength{\parindent}{0pt}%
  \noindent(#1)\quad #2\par}%
}
\newcommand{\blueitemline}[2]{%
  {\setlength{\leftskip}{2em}\setlength{\parindent}{0pt}\color{blue}%
  \noindent(#1)\quad #2\par}%
}
\newcommand{\spacedblueitemline}[2]{%
  \blueitemline{#1}{#2}\vspace{0.88cm}%
}
\newcommand{\lineblueitemline}[2]{%
  \blueitemline{#1}{#2}\vspace{0.18cm}%
}

\newcommand{\makefrontmatter}{%
  \thispagestyle{empty}%
  \zihao{5}%
  \noindent
  {\setlength{\tabcolsep}{2pt}
  \begin{tabular}{|C{1.65cm}|C{3.80cm}|C{3.80cm}|}
  \hline
  \multicolumn{3}{|c|}{\heiti 总成绩（30 分）}\\
  \hline
  \multirowcell{2}{\heiti 项目} & \heiti 通讯协议实现及分析 & \heiti 报告质量及团队协作\\
   & \heiti 15 分 & \heiti 15 分\\
  \hline
  \heiti 得分 & & \\
  \hline
  \heiti 评分人 & \multicolumn{2}{c|}{}\\
  \hline
  \end{tabular}
  }\par
  \vspace{1.20cm}%
  \begin{center}
    \cqutCoverLogoHorizontal[10.4cm]\par
    \vspace{1.35cm}
    {\heiti\bfseries\zihao{-0} 综合课程设计I 报告\par}
    \vspace{1.05cm}
    {\heiti\zihao{3}题目：\quad \underline{\makebox[8.0cm][c]{桌面用电监控设计实现}}\par}
    \vspace{1.65cm}
    \begin{minipage}{9.8cm}
      \zihao{4}
      \coverrow{二级学院}{7.0cm}{计算机科学与工程学院}{0pt}
      \coverrow{专\qquad 业}{7.0cm}{物联网工程}{0pt}
      \coverrow{班\qquad 级}{7.0cm}{}{0pt}
      \noindent 学生姓名\underline{\makebox[3.0cm][c]{}}学号\underline{\makebox[2.9cm][c]{}}\\[0.42cm]
      \coverrow{指导教师}{7.0cm}{}{0pt}
      \coverrow{时\qquad 间}{7.0cm}{2026.6}{0pt}
    \end{minipage}
  \end{center}
  \clearpage
  \thispagestyle{empty}%
  \vspace*{2.95cm}%
  \zihao{-4}%
  \setlength{\parindent}{0pt}%
  \rednote{双面打印，本页打印空白页，本页及后续的格式说明文字不打印}\par
  \vspace{0.45cm}%
  \rednote{格式说明：}\par
  \vspace{0.25cm}%
  \rednote{1、文件名统一按“学号-姓名-综合课程设计I报告.doc”命名，比如：}\par
  \vspace{0.25cm}%
  \hspace{2em}\rednote{12403010199-张三-综合课程设计I报告.doc；}\par
  \vspace{0.25cm}%
  \rednote{2、正文中图、表、公式需要按格式编号；}\par
  \vspace{0.25cm}%
  \rednote{3、目录及内容格式如下}\par
  \clearpage
  \thispagestyle{empty}%
  \mbox{}%
  \clearpage
  \pagenumbering{Roman}%
  \setcounter{page}{1}%
  \thispagestyle{empty}%
  \setlength{\parindent}{2em}%
  \vspace*{7.05cm}%
  \phantomsection\label{sec:abstract}%
  \begin{center}
  {\heiti\zihao{4} 摘\quad 要}
  \end{center}
  \vspace{0.45cm}%
  \zihao{-4}%
  \rednote{（正文为宋体五号，20 磅行距，图片须有下标且小一号字体，文档完成后，点击“更新目录”，形成对应的目录表，双面打印）}\par
  \vspace{1.55cm}%
  \noindent{\heiti 关键词：}（3-5 个）
  \clearpage
  \thispagestyle{empty}%
  \mbox{}%
  \clearpage
  \thispagestyle{reporthead}%
  \vspace*{0.45cm}%
  \begin{center}
  {\heiti\zihao{3} 目\quad 录}
  \end{center}
  \vspace{0.58cm}%
  \zihao{5}%
  \setlength{\parindent}{0pt}%
  \tocline{摘\quad 要}{\pageref{sec:abstract}}%
  \tocline{1. 课程设计的目的及要求}{\pageref{sec:purpose}}%
  \tocsubline{1.1. 课程设计目的}{\pageref{sec:purpose-goal}}%
  \tocsubline{1.2. 课程设计要求}{\pageref{sec:purpose-req}}%
  \tocline{2. 系统分析}{\pageref{sec:analysis}}%
  \tocsubline{2.1. 功能分析}{\pageref{sec:analysis-function}}%
  \tocsubline{2.2. 性能需求分析}{\pageref{sec:analysis-performance}}%
  \tocsubline{2.3. 算法分析}{\pageref{sec:analysis-algorithm}}%
  \tocline{3. 系统设计}{\pageref{sec:design}}%
  \tocsubline{3.1. 系统整体设计}{\pageref{sec:design-system}}%
  \tocsubline{3.2. 硬件设计}{\pageref{sec:design-hardware}}%
  \tocsubline{3.3. 软件设计}{\pageref{sec:design-software}}%
  \tocline{4. 频率测量}{\pageref{sec:frequency}}%
  \tocsubline{4.1. 测试频率生成}{\pageref{sec:frequency-generate}}%
  \tocsubline{4.2. 频率测量}{\pageref{sec:frequency-measure}}%
  \tocsubline{4.3. 通讯协议}{\pageref{sec:frequency-protocol}}%
  \tocline{5. 交流 ADC 采样}{\pageref{sec:adc}}%
  \tocsubline{5.1. 测试电压电流生成}{\pageref{sec:adc-generate}}%
  \tocsubline{5.2. 信号采集}{\pageref{sec:adc-sampling}}%
  \tocsubline{5.3. 通信协议}{\pageref{sec:adc-protocol}}%
  \tocsubline{5.4. 波形曲线绘制}{\pageref{sec:adc-waveform}}%
  \tocline{6. 电参数计算与分析}{\pageref{sec:calculation}}%
  \tocsubline{6.1. 常见电参数计算方法}{\pageref{sec:calculation-method}}%
  \tocsubline{6.2. 程序实现}{\pageref{sec:calculation-program}}%
  \tocsubline{6.3. 通信设计}{\pageref{sec:calculation-comm}}%
  \tocline{7. 系统调试}{\pageref{sec:debug}}%
  \tocsubline{7.1. 通信协议分析}{\pageref{sec:debug-protocol}}%
  \tocsubline{7.2. 频率调试}{\pageref{sec:debug-frequency}}%
  \tocsubline{7.3. AD 采样调试}{\pageref{sec:debug-ad}}%
  \tocsubline{7.4. 电参数调试}{\pageref{sec:debug-calculation}}%
  \tocsubline{7.5. 联机调试}{\pageref{sec:debug-online}}%
  \tocline{8. 总结与展望}{\pageref{sec:summary}}%
  \tocline{附件：程序代码}{\pageref{sec:appendix}}%
  \clearpage
  \thispagestyle{empty}%
  \mbox{}%
  \clearpage
}

% 课程 Word：宋体五号、约 20 磅行距
\let\cqut@savedBeginReportBody\cqutBeginReportBody
\renewcommand{\cqutBeginReportBody}{%
  \pagestyle{reportbody}%
  \pagenumbering{arabic}%
  \cqut@savedBeginReportBody
  \zihao{5}%
  \setlength{\parindent}{2em}%
  \linespread{1.18}\selectfont
}

\endinput
