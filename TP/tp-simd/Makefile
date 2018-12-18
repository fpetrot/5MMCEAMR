LATEX=pdflatex
BIBTEX=bibtex

ALLTEX := $(wildcard *.tex)
ALLSTY := $(wildcard *.cls)

ALLSVG := $(wildcard ./svg/*.svg) $(wildcard ./graphs/*.svg)
ALLPDF := $(ALLSVG:%.svg=%.pdf)

.PHONY : clean realclean view git-archive archive

PAPER := tp-simd

$(PAPER).pdf: $(ALLTEX) $(ALLBIB) $(ALLSTY) $(ALLPDF)
	$(LATEX) $(PAPER).tex  # First compilation
	$(LATEX) $(PAPER).tex  # Second compilation

./svg/%.pdf: ./svg/%.svg
	inkscape -z -T -D --export-pdf=$@ --file=$<

./graphs/%.pdf: ./graphs/%.svg
	inkscape -z -T -D --export-pdf=$@ --file=$<

clean:
	rm -f *.aux *.bbl *.blg *.lof *.log *.lot *.out *.toc

realclean: clean
	rm -f *.pdf ./svg/*.pdf

view: $(PAPER).pdf
	evince $(PAPER).pdf &

git-archive:
	git archive -o compas17.tar.gz master

archive:
	tar -zcvf *.tar.gz Makefile *.tex *.pdf *.bst *.cls *.bib fig/*.svg fig/*.pdf
