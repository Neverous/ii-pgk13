BUILDDIR=build

debug: clean
	cd ${BUILDDIR} && \
	cmake -DCMAKE_BUILD_TYPE=Debug .. && \
	make VERBOSE=1;

release: clean
	cd ${BUILDDIR} && \
	cmake -DCMAKE_BUILD_TYPE=Release .. && \
	make VERBOSE=1;

static-debug: clean
	cd ${BUILDDIR} && \
	cmake -DUSE_STATIC=Yes -DCMAKE_BUILD_TYPE=Debug .. && \
	make VERBOSE=1;

static-release: clean
	cd ${BUILDDIR} && \
	cmake -DUSE_STATIC=Yes -DCMAKE_BUILD_TYPE=Release .. && \
	make VERBOSE=1;

clean:
	-rm -rf ${BUILDDIR};
	mkdir -p ${BUILDDIR};
