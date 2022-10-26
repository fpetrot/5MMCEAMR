PDFLATEX=pdflatex --shell-escape
SVG=$(wildcard figures/*.svg)
PDF=$(SVG:%.svg=%.pdf)

.PHONY: all clean

%.pdf : %.svg
	inkscape --export-filename=$@ $<

all: tp-branch.pdf

tp-branch.pdf : tp-branch.tex $(PDF)
	$(PDFLATEX) $<

clean :
	$(RM) tp-branch.pdf
