// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <clk.h>
#include <scmi.h>
#include <dm/uclass.h>

#define SCMI_CLOCK_NAME_LENGTH		16
#define SCMI_NUM_CLOCKS_MASK		0xFF
#define SCMI_ENABLED_MASK		0x1
#define SCMI_MID_PROTOCOL_ATTRIBUTES	0x1
#define SCMI_MID_CLOCK_ATTRIBUTES	0x3
#define SCMI_MID_CLOCK_RATE_GET		0x6

static int send_scmi_msg(struct scmi_msg *msg)
{
	int ret;
	struct udevice *scmi = NULL;

	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "scmi", &scmi);
	if (ret)
		return ret;

	return scmi_send_and_process_msg(scmi, msg);
}

static int get_num_clocks(void)
{
	int ret;
	struct {
		s32 status;
		u32 attributes;
	} response;

	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_CLOCK,
		.message_id	= SCMI_MID_PROTOCOL_ATTRIBUTES,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};

	ret = send_scmi_msg(&msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		return ret;

	return response.attributes & SCMI_NUM_CLOCKS_MASK;
}

static int get_clock_info(int clock_index, char *clock_name, int *rate,
			  int *enabled)
{
	int ret;
	struct {
		u32 clock_id;
	} input_msg;
	struct {
		s32 status;
		u32 attributes;
		u8 clock_name[SCMI_CLOCK_NAME_LENGTH];
	} response_clock_attributes;
	struct {
		s32 status;
		u32 rate_lower;
		u32 rate_upper;
	} response_clock_rate_get;

	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_CLOCK,
		.message_id	= SCMI_MID_CLOCK_ATTRIBUTES,
		.in_msg		= (u8 *)&input_msg,
		.in_msg_sz	= sizeof(input_msg),
		.out_msg	= (u8 *)&response_clock_attributes,
		.out_msg_sz	= sizeof(response_clock_attributes)
	};

	input_msg.clock_id = clock_index;
	memset(response_clock_attributes.clock_name, 0, SCMI_CLOCK_NAME_LENGTH);

	ret = send_scmi_msg(&msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response_clock_attributes.status);
	if (ret)
		return ret;

	msg.message_id = SCMI_MID_CLOCK_RATE_GET;
	msg.out_msg	= (u8 *)&response_clock_rate_get;
	msg.out_msg_sz	= sizeof(response_clock_rate_get);

	ret = send_scmi_msg(&msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response_clock_rate_get.status);
	if (ret)
		return ret;

	memcpy(clock_name, response_clock_attributes.clock_name,
	       SCMI_CLOCK_NAME_LENGTH);
	*enabled = response_clock_attributes.attributes & SCMI_ENABLED_MASK;
	*rate = (u64)response_clock_rate_get.rate_upper << 32 |
		response_clock_rate_get.rate_lower;

	return 0;
}

int soc_clk_dump(void)
{
	int ret;
	int num_clocks;
	int i;
	int enabled;
	char clock_name[SCMI_CLOCK_NAME_LENGTH + 1];
	int clock_rate;

	num_clocks = get_num_clocks();
	if (num_clocks < 0) {
		printf("Error retrieving the number of clocks!\n");
		return num_clocks;
	}

	printf(" Rate               Used         Name\n");
	printf("-------------------------------------------\n");

	for (i = 0; i < num_clocks; ++i) {
		memset(clock_name, 0, ARRAY_SIZE(clock_name));

		ret = get_clock_info(i, clock_name, &clock_rate, &enabled);
		if (ret) {
			printf("Error retrieving info for clock: %d!\n", i);
			continue;
		}

		printf(" %-12u  %8d          ", clock_rate, enabled);
		printf("%s\n", clock_name);
	}

	return 0;
}
