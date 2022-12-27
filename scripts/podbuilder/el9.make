BUILDER=el9
BUILDER_IMG=mistbuilder-$(BUILDER)
BUILDER_CTR=$(BUILDER_IMG)c
BUILDER_DOCKERFILE=./$(BUILDER).dockerfile

SOURCE_DIR=../../
BUILD_DIR= build.$(BUILDER)
BUILD_BIN=$(BUILD_DIR).bin

CTR_SOURCE_DIR=/home/src
CTR_BUILD_DIR=/home/build
CTR_OUT_DIR=/home/out
CTR_RUN_COMMAND=
CTR_RUN_OPTIONS=
ifdef DEBUG
CTR_RUN_COMMAND=/bin/bash
CTR_RUN_OPTIONS=-it
endif


build ::
	-@podman rm --force --ignore $(BUILDER_CTR)
	podman build --pull --rm --tag $(BUILDER_IMG) --file ./$(BUILDER_DOCKERFILE)
	podman run $(CTR_RUN_OPTIONS) \
		-v $(SOURCE_DIR):$(CTR_SOURCE_DIR):ro,Z \
		--name $(BUILDER_CTR) \
		-v .:$(CTR_OUT_DIR):Z \
		$(BUILDER_IMG) $(CTR_RUN_COMMAND)

BUILD_VERSION := $(shell git describe --tags | cut -d- -f1 | tr -d v)
BUILD_RELEASE := $(shell git describe --tags | cut -d- -f2).$(shell git log --pretty=format:'%h' -n 1)
ctr-build:
	rpmbuild \
		--define "_topdir `mktemp -d`" \
		--define "_builddir /home/build" \
		--define "_sourcedir /home/src" \
		--define "_rpmdir /home/out" \
		--define "build_version $(BUILD_VERSION)" \
		--define "release $(BUILD_RELEASE)" \
		-ba el9.spec
	
ctr-clean:
	rm -rf $(CTR_BUILD_DIR)

clean ::
	-@podman container rm --force $(BUILDER_CTR)

distclean ::
	-@podman image rm --force $(BUILDER_IMG)
	-@podman image prune --force
	-@podman container rm --force $(BUILDER_CTR)
	-@podman container prune --force
	-@rm -f *.rpm
