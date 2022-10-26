PDFLATEX=pdflatex --shell-escape
.PHONY: all clean
all tp-branch.pdf : tp-branch.tex
	$(PDFLATEX) $<

clean:
	$(RM) tp-branch.pdf
