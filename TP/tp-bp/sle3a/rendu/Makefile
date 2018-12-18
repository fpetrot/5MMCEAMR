SVG := $(wildcard ../figures-*/*.svg)
SVG2PDF=$(SVG:.svg=.pdf)

%.pdf: %.svg
	@echo Converting $< to $@
	@inkscape -T --file=$< --export-pdf=$@ 2> /dev/null
tp-branch-res.pdf : tp-branch-res.tex ${SVG2PDF}
	pdflatex $<
clean :
	rm -f tp-branch-res.pdf tp-branch-res.aux  tp-branch-res.log
