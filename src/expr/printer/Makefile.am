AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/src/expr	\
-I$(top_srcdir)/src/expr/walker -I$(top_srcdir)/src/model	\
-I$(top_srcdir)/src/3rdparty/ezlogger				\
-I$(top_srcdir)/src/dd/cudd-2.5.0/cudd				\
-I$(top_srcdir)/src/dd/cudd-2.5.0/mtr				\
-I$(top_srcdir)/src/dd/cudd-2.5.0/st				\
-I$(top_srcdir)/src/dd/cudd-2.5.0/util				\
-I$(top_srcdir)/src/dd/cudd-2.5.0/obj

PKG_HH = printer.hh
PKG_CC = leaf.cc printer.cc

# -------------------------------------------------------

noinst_LTLIBRARIES = libprinter.la
libprinter_la_SOURCES = $(PKG_HH) $(PKG_CC)
