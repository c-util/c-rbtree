#
# Shared Libraries
#
# This makefile implements $(SOLIBRARIES). It is modeled around the
# libtool-style $(LTLIBRARIES) support, but does not depend on libtool.
#
# How to use:
#     If you have a library libfoo.a that you want to provide as .so, just add
#     its SONAME to $(SOLIBRARIES). How libfoo.a is built is up to you. Usually
#     you would make use of noinst_LIBRARIES like this:
#
#         libfoo_a_SOURCES = libfoo.h libfoo.c
#         include_HEADERS = libfoo.h
#         noinst_LIBRARIES = libfoo.a
#         SOLIBRARIES = libfoo.so.0
#
#     Note that your SONAME must adhere to this template:
#         $(NAME).so.$(SOVERSION)
#     That is, the SONAME is prefixed by the same name as your archive, and
#     suffixed by the SOVERSION.
#
#     A linker-script (i.e., symbol-file) must be available as $(NAME).sym.
#
# When adding your library to $(SOLIBRARIES), a shared object with the given
# SONAME is built, as well as integrated into `make install`. By default, an
# '.so' -> '.so.$(SOVERSION)' symlink is installed as well.
#
# Please note that this integrates with automake. That is, the build is hooked
# into CLEANFILES, {all,install-exec,uninstall}-local, EXTRA_DIST, etc.
# Furthermore, if automake is used, this will also require a pkg-config file to
# be available as $(NAME).pc.in.
#

#
# Helpers to extract information from $(SOLIBRARIES):
#   $(call so_basename,libfoo.so.10) -> libfoo
#   $(call so_version,libbar.so.10) -> 10
#   $(so_list) -> libfoo libbar
#
so_basename = $(basename $(basename $(1)))
so_version = $(patsubst $(basename $(1)).%,%,$(1))
so_list = $(call so_basename,$(SOLIBRARIES))

#
# Build-rule template for .so, .pc., and install/uninstall hooks. Since the
# template uses prefix+suffix matching, we cannot use generic '%'-implicit
# rules. Instead, we have to generate the rules via $(eval ...) based on this
# template.
#
define so_template =
    src/$(1).pc: src/$(1).pc.in
	$$(AM_V_GEN)$$(SED) \
		-e 's,@includedir\@,$$(includedir),g' \
		-e 's,@libdir\@,$$(libdir),g' \
		-e 's,@VERSION\@,$$(VERSION),g' \
		$$< > $$@
    $(1).so.$(2): src/$(1).sym $(1).a
	$$(AM_V_CCLD)$$(LINK) \
		-shared \
		-Wl,-soname=$$@ \
		-Wl,--version-script=$$< \
		-Wl,--whole-archive $(1).a -Wl,--no-whole-archive
    so-install-$(1).so.$(2):
	@echo " $$(MKDIR_P) '$$(DESTDIR)$$(libdir)'"; \
		$$(MKDIR_P) "$$(DESTDIR)$$(libdir)" || exit 1
	@echo " $$(INSTALL) $(1).so.$(2) '$$(DESTDIR)$$(libdir)'"; \
		$$(INSTALL) $(1).so.$(2) "$$(DESTDIR)$$(libdir)" || exit $$?
	@echo " $$(LN_S) -f $(1).so.$(2) '$$(DESTDIR)$$(libdir)/$(1).so'"; \
		$$(LN_S) -f $(1).so.$(2) "$$(DESTDIR)$$(libdir)/$(1).so" || exit $$?
    so-uninstall-$(1).so.$(2):
	@test ! -d "$$(DESTDIR)$$(libdir)" || \
		{ echo " ( cd '$$(DESTDIR)$$(libdir)' && rm -f $(1).so )"; \
		  rm -f "$$(DESTDIR)$$(libdir)/$(1).so"; }
	@test ! -d "$$(DESTDIR)$$(libdir)" || \
		{ echo " ( cd '$$(DESTDIR)$$(libdir)' && rm -f $(1).so.$(2) )"; \
		  rm -f "$$(DESTDIR)$$(libdir)/$(1).so.$(2)"; }
    .PHONY: so-install-$(1).so.$(2) so-uninstall-$(1).so.$(2)
endef

#
# Generate one rule-set from so_template for each entry specified in
# $(SOLIBRARIES). Note that $(so_template) takes the basename as first argument
# and the version as second argument.
#
$(foreach soname,$(SOLIBRARIES),$(eval $(call so_template,$(call so_basename,$(soname)),$(call so_version,$(soname)))))

#
# Integrate with automake. Since automake pre-parses these variables, we cannot
# generate them as part of $(so_template). Instead, we have to explicitly
# assign them.
#
all-local: $(SOLIBRARIES)
install-exec-local: $(addprefix so-install-,$(SOLIBRARIES))
uninstall-local: $(addprefix so-uninstall-,$(SOLIBRARIES))
so_pkgconfiglibdir = $(libdir)/pkgconfig
so_pkgconfiglib_DATA = $(patsubst %,src/%.pc,$(so_list))
CLEANFILES += \
	$(SOLIBRARIES) \
	$(patsubst %,src/%.pc,$(so_list))
EXTRA_DIST += \
	$(patsubst %,src/%.pc.in,$(so_list)) \
	$(patsubst %,src/%.sym,$(so_list))
