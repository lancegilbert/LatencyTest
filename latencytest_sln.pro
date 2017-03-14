cache()

TEMPLATE = subdirs

SUBDIRS += latencytest external

latencytest.depends = external
