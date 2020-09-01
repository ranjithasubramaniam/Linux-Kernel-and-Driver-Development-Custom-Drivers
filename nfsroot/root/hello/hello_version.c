// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/utsname.h>
#include <generated/utsrelease.h>
#include <linux/timekeeping32.h>

/* Add your code here */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A Hello World program to try out Kernel Modules");
MODULE_AUTHOR("Suresh");

static unsigned long loadTime;
// A variable to hold the time when the module is loaded


static char *whom = "world";
module_param(whom, charp, 0644);
MODULE_PARM_DESC(whom, "Recipient of the hello message");

static int __init hello_init(void)
{
	loadTime = get_seconds();
	pr_alert("Hello %s. You are currently using Linux %s.\n"
		 , whom, utsname()->release);
	pr_alert("Hello %s. You are currently using Linux compiled with version %s.\n"
		 , whom, UTS_RELEASE);
	return 0;
}

static void __exit hello_exit(void)
{
	pr_alert("Alas, poor %s, what treasure hast thou lost in %ld!\n",
		 whom, get_seconds() - loadTime);
}


module_init(hello_init);
module_exit(hello_exit);
