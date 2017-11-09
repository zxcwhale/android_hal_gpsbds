#Androidv2.2 does not support SUPL
ifneq ($(PLATFORM_SDK_VERSION),8)
SUPL_ENABLED := 1
endif
include $(all-subdir-makefiles)
