.PHONY  : all
INKSCAPE= inkscape
PDFLATEX = rubber --pdf

DIAGRAM := $(wildcard figures/*.svg) 
FIGURES = $(DIAGRAM:%.svg=%.pdf)

%.pdf : %.svg
	$(INKSCAPE) -T --file=$< --export-pdf=$@

tp-branch.pdf : tp-branch.tex $(FIGURES)
	$(PDFLATEX) $<

clean:
	$(RM) *aux *log *nav *out *snv *toc *snm *dvi *bak *vrb

realclean: clean
	$(RM) tp-branch.pdf figures/*.pdf
