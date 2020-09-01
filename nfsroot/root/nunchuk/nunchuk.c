// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/input-polldev.h>
/* Add your code here */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A device driver for Wii Nunchuk as I2C device");
MODULE_AUTHOR("Suresh");

/* Global Declarations */
struct nunchuk_dev {
	struct i2c_client *i2c_client;
};

/* Function to read nunchuk data*/
static int nunchuk_read_registers(struct i2c_client *client, u8 *buf)
{
		int status;
		int error;
		
		u8 cmd_read = 0x00;
		
		usleep_range(10,20);
		
		/* Send the read command 0x00 */
		
		status =  i2c_master_send(client, &cmd_read, sizeof(cmd_read));
		
		if (status != sizeof(cmd_read))
		{	
			error = status < 0 ? status : -EIO;
			dev_err(&client->dev, "Error during nunchuk read: failed to send the read command %d", error);	
			return error;
		}
		else 
		{
			dev_info(&client->dev, "Nunchuk Read: Successfully sent read command %d", status);
		} 

			
		usleep_range(20, 30);	
		
		status = i2c_master_recv(client, buf, 6);


		if (status != 6)
		{	
			error = status < 0 ? status : -EIO;
			dev_err(&client->dev, "Error during nunchuk read: failed to read the nunchuk data %d", error);	
			return error;
		}
		else 
		{
			dev_info(&client->dev, "Nunchuk Read: Successfully read nunchuk data %d", status);
		} 
		
		return 0;
}


/* Function to poll nunchuk registers periodically*/
static void nunchuk_poll(struct input_polled_dev *polled_input)
{		
		enum nunchuk_bitinfo_byte_6
		{
			Z_BIT = 0, C_BIT = 1
		};
		
		enum nunchuk_byteinfo
		{
			BYTE0,  BYTE1,  BYTE2,  BYTE3,  BYTE4,  BYTE5    
		};
		
		//create the buffer
		u8 buf[6] = {0};
		int zpressed = 0;
		int cpressed = 0;
		int status;
		int error;
		
		// get the physical device i.e. nunchuk	    
		struct i2c_client *client = ((struct nunchuk_dev *)polled_input->private)->i2c_client;
		
		
		/* Routine for nunchuk_read_registers */
		u8 cmd_read = 0x00;
		
		usleep_range(10,20);
		
		/* Send the read command 0x00 */
		
		status =  i2c_master_send(client, &cmd_read, sizeof(cmd_read));
		
		if (status != sizeof(cmd_read))
		{	
			error = status < 0 ? status : -EIO;
			dev_err(&client->dev, "Error during nunchuk read: failed to send the read command %d", error);			
			return;	
		}
		else 
		{
			dev_info(&client->dev, "Nunchuk Read: Successfully sent read command %d", status);
		} 

			
		usleep_range(20, 30);	
		
		status = i2c_master_recv(client, buf, 6);


		if (status != 6)
		{	
			error = status < 0 ? status : -EIO;
			dev_err(&client->dev, "Error during nunchuk read: failed to read the nunchuk data %d", error);			
			return;	
		}
		else 
		{
			dev_info(&client->dev, "Nunchuk Read: Successfully read nunchuk data %d", status);
		} 
		
		/* End of routine for nunchuk_read_registers */
				
		zpressed = (*(buf + BYTE5) & (1<<Z_BIT)) >> Z_BIT;
	    cpressed = (*(buf + BYTE5) & (1<<C_BIT)) >> C_BIT;
		
		// notify the events to the input core
		input_event(polled_input->input, EV_KEY, BTN_Z, zpressed);
		input_event(polled_input->input, EV_KEY, BTN_C, cpressed);
		// notify the input core after submitting multiple events						
		input_sync(polled_input->input);								


	
}

static int nunchuk_i2c_probe(struct i2c_client *client,
									 const struct i2c_device_id *id)
{
	int status;
	
	struct handshake_signal {
		u8 firstByte;
		u8 secondByte;
	};
	
	/*
	enum nunchuk_bitinfo_byte_6
	{
		Z_BIT, C_BIT
	};
	
	enum nunchuk_byteinfo
	{
		BYTE0,  BYTE1,  BYTE2,  BYTE3,  BYTE4,  BYTE5    
	};
	
	int zpressed = 0;
	int cpressed = 0;
	u8 buf[6] = {0};
	
	*/
	
	struct handshake_signal init_register_1 = {0xf0, 0x55};
	struct handshake_signal init_register_2 = {0xfb, 0x00};
	
	/* Input subsystem related declarations */
	
	struct input_polled_dev *polled_input;
	struct input_dev *input;
	struct nunchuk_dev *nunchuk;
	
	/* End of Input subsystem related declarations */
	
	pr_alert("The probe function is called");
	
	/* Input subsystem related functions */
	
	//allocate memory for nunchuk_dev
	nunchuk = devm_kzalloc(&client->dev, sizeof(struct nunchuk_dev), GFP_KERNEL);
	if (!nunchuk) {
		return -ENOMEM;
	}
	
	// assign the existing client to the nunchuk_dev -> client
	nunchuk->i2c_client = client;
		
	// allocate memory for polled input device 
	polled_input = devm_input_allocate_polled_device(&client->dev);
	
	// create a link between framework and device
	polled_input->private = nunchuk;
	
	// initialise the struct input inside the polled device structure
	input = polled_input->input;
	input->name = "Wii Nunchuk";
	input->id.bustype = BUS_I2C;
	
	set_bit(EV_KEY, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);
	
	// assign the poll function
	polled_input->poll = nunchuk_poll;
	
	// assign the polling intervel as 50 ms
	polled_input->poll_interval = 50;
	
	// register the polled input device 
	status = input_register_polled_device(polled_input);
	
	if (status)
	{
		dev_err(&client->dev, "Error during polled input device registration: failed to register the polled device");	
		return status;
	}
	else 
	{
		dev_info(&client->dev, "Polled Input Device Registration : Successfully registered the polled input device");	
	}
	
	/* End of Input subsystem related functions */
	
	
	//Send the handshake signal and initialise the first register 
		
	status =  i2c_master_send(client, &init_register_1.firstByte, sizeof(init_register_1));
	
	if (status < 0)
	{
		dev_err(&client->dev, "Error during handshake: failed to initialise the first register");	
		return status;
	}
	else 
	{
		dev_info(&client->dev, "Handshake: Successfully initialised the first register");	
	}
	
	udelay(1000);
	
	// Send the handshake signal and initialise the second register
	
	status =  i2c_master_send(client, &init_register_2.firstByte, sizeof(init_register_2));
	
	if (status < 0)
	{
		dev_err(&client->dev, "Error during handshake: failed to initialise the second register");	
		return status;
	}
	else 
	{
		dev_info(&client->dev, "Handshake: Successfully initialised the second register");	
	}
	
	/*
 	// get the nunchuk register data 
	status = nunchuk_read_registers(client, buf);
	status = nunchuk_read_registers(client, buf);
	
	//The sixth byte of the buffer contains C and Z button status
	
	zpressed = *(buf + BYTE5) & (1<<Z_BIT);
	cpressed = *(buf + BYTE5) & (1<<C_BIT);
	
	if (!zpressed)
		dev_info(&client->dev, "Button Z is pressed");		
	else 
		dev_info(&client->dev, "Button Z is not pressed");
	
	if (!cpressed)
		dev_info(&client->dev, "Button C is pressed");		
	else 
		dev_info(&client->dev, "Button C is not pressed"); 
	*/
	
	
	return 0;	
}

static int nunchuk_i2c_remove(struct i2c_client *client)
{
	dev_info(&client->dev, "The remove function is called");
	return 0;
}

static const struct i2c_device_id nunchuk_i2c_id[] = {
	{"nunchuk", 1 },
	{}
};

MODULE_DEVICE_TABLE(i2c, nunchuk_i2c_id);


static const struct of_device_id nunchuk_of_match[] = {
	{ .compatible = "nintendo,nunchuk" },
	{ },
};

MODULE_DEVICE_TABLE(of, nunchuk_of_match);


static struct i2c_driver nunchuk_i2c_driver = {
	.probe = nunchuk_i2c_probe,
	.remove = nunchuk_i2c_remove,
	.id_table = nunchuk_i2c_id,
	.driver = {
		.name = "nunchuk_i2c",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(nunchuk_of_match),
	},
};

module_i2c_driver(nunchuk_i2c_driver);