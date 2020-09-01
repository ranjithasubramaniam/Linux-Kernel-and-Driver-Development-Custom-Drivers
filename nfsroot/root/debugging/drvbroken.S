
drvbroken.o:     file format elf32-littlearm


Disassembly of section .text:

00000000 <broken_write>:
static struct cdev *broken_cdev;

static ssize_t
broken_write(struct file *file, const char __user *buf, size_t count,
	     loff_t *ppos)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e1a03002 	mov	r3, r2
	pr_info("Writing %d bytes from %p to the device\n",
  10:	e59f0010 	ldr	r0, [pc, #16]	; 28 <broken_write+0x28>
  14:	e1a02001 	mov	r2, r1
  18:	e1a01003 	mov	r1, r3
  1c:	ebfffffe 	bl	0 <printk>
	      count, buf);
	return 0;
}
  20:	e3a00000 	mov	r0, #0
  24:	e89da800 	ldm	sp, {fp, sp, pc}
  28:	00000000 	.word	0x00000000

0000002c <broken_read>:

static ssize_t
broken_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
  2c:	e1a0c00d 	mov	ip, sp
  30:	e92dd800 	push	{fp, ip, lr, pc}
  34:	e24cb004 	sub	fp, ip, #4
  38:	e1a03002 	mov	r3, r2
	pr_info("Writing %d bytes to %p from the device\n",
  3c:	e59f0010 	ldr	r0, [pc, #16]	; 54 <broken_read+0x28>
  40:	e1a02001 	mov	r2, r1
  44:	e1a01003 	mov	r1, r3
  48:	ebfffffe 	bl	0 <printk>
	count, buf);
	return 0;
}
  4c:	e3a00000 	mov	r0, #0
  50:	e89da800 	ldm	sp, {fp, sp, pc}
  54:	0000002c 	.word	0x0000002c

Disassembly of section .init.text:

00000000 <broken_init>:
	.read = broken_read,
	.write = broken_write,
};

int __init broken_init(void)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	if (alloc_chrdev_region(&broken_dev, broken_first_minor, 1, "broken") <
   c:	e59f4088 	ldr	r4, [pc, #136]	; 9c <broken_init+0x9c>
  10:	e59f3088 	ldr	r3, [pc, #136]	; a0 <broken_init+0xa0>
  14:	e1a00004 	mov	r0, r4
  18:	e3a02001 	mov	r2, #1
  1c:	e3a01000 	mov	r1, #0
  20:	ebfffffe 	bl	0 <alloc_chrdev_region>
  24:	e3500000 	cmp	r0, #0
  28:	aa000003 	bge	3c <broken_init+0x3c>
	    0) {
		pr_err("broken: unable to find free device numbers\n");
  2c:	e59f0070 	ldr	r0, [pc, #112]	; a4 <broken_init+0xa4>
  30:	ebfffffe 	bl	0 <printk>
		return -EIO;
  34:	e3e00004 	mvn	r0, #4
  38:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
	}

	cdev_init(broken_cdev, &broken_fops);
  3c:	e59f1064 	ldr	r1, [pc, #100]	; a8 <broken_init+0xa8>
  40:	e3a00000 	mov	r0, #0
  44:	ebfffffe 	bl	0 <cdev_init>

	if (cdev_add(broken_cdev, broken_dev, 1) < 0) {
  48:	e3a02001 	mov	r2, #1
  4c:	e5941000 	ldr	r1, [r4]
  50:	e3a00000 	mov	r0, #0
  54:	ebfffffe 	bl	0 <cdev_add>
  58:	e3500000 	cmp	r0, #0
  5c:	aa000006 	bge	7c <broken_init+0x7c>
		pr_err("broken: unable to add a character device\n");
  60:	e59f0044 	ldr	r0, [pc, #68]	; ac <broken_init+0xac>
  64:	ebfffffe 	bl	0 <printk>
		unregister_chrdev_region(broken_dev, broken_count);
  68:	e5940000 	ldr	r0, [r4]
  6c:	e3a01001 	mov	r1, #1
  70:	ebfffffe 	bl	0 <unregister_chrdev_region>
  74:	e3e00004 	mvn	r0, #4
  78:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
		return -EIO;
	}

	pr_info("Loaded the broken driver: major = %d, minor = %d\n",
  7c:	e5941000 	ldr	r1, [r4]
  80:	e59f0028 	ldr	r0, [pc, #40]	; b0 <broken_init+0xb0>
  84:	e3c124ff 	bic	r2, r1, #-16777216	; 0xff000000
  88:	e3c2260f 	bic	r2, r2, #15728640	; 0xf00000
  8c:	e1a01a21 	lsr	r1, r1, #20
  90:	ebfffffe 	bl	0 <printk>
	       MAJOR(broken_dev), MINOR(broken_dev));

	return 0;
  94:	e3a00000 	mov	r0, #0
}
  98:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
  9c:	00000000 	.word	0x00000000
  a0:	00000058 	.word	0x00000058
  a4:	00000060 	.word	0x00000060
  a8:	00000000 	.word	0x00000000
  ac:	00000090 	.word	0x00000090
  b0:	000000bc 	.word	0x000000bc

Disassembly of section .exit.text:

00000000 <broken_exit>:

void __exit broken_exit(void)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd800 	push	{fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	cdev_del(broken_cdev);
   c:	e3a00000 	mov	r0, #0
  10:	ebfffffe 	bl	0 <cdev_del>
	unregister_chrdev_region(broken_dev, broken_count);
  14:	e59f3014 	ldr	r3, [pc, #20]	; 30 <broken_exit+0x30>
  18:	e3a01001 	mov	r1, #1
  1c:	e5930000 	ldr	r0, [r3]
  20:	ebfffffe 	bl	0 <unregister_chrdev_region>
	pr_info("Unloaded the broken driver!\n");
  24:	e59f0008 	ldr	r0, [pc, #8]	; 34 <broken_exit+0x34>
  28:	ebfffffe 	bl	0 <printk>
}
  2c:	e89da800 	ldm	sp, {fp, sp, pc}
  30:	00000000 	.word	0x00000000
  34:	000000f0 	.word	0x000000f0
