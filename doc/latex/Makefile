outfiles=dataformat.pdf architecture.pdf

all: $(outfiles)

$(outfiles): %.pdf: %.tex customdocclass.cls
	mkdir -p build
	lualatex -interaction=batchmode --output-directory=build $<
	cp build/$@ .
	rm -r build/

.PHONY: clean
clean:
	rm -rf *.pdf
