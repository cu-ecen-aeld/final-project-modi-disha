
##############################################################
#
# GPIOMODULE-ASSIGNMENTS
#
##############################################################

#TODO: Fill up the contents below in order to reference your assignment 3 git contents
GPIOMODULE_VERSION = 61fc53bc6a2875e78e80c28c95e8da7fc300123a


# Note: Be sure to reference the *ssh* repository URL here (not https) to work properly
# with ssh keys and the automated build/test system.
# Your site should start with git@github.com:
GPIOMODULE_SITE=git@github.com:cu-ecen-5013/final-project-modi-disha.git
GPIOMODULE_SITE_METHOD = git
GPIOMODULE_MODULE_SUBDIRS = aesd-gpio-driver/

$(eval $(kernel-module))
$(eval $(generic-package))
