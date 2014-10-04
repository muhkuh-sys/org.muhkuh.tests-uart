module("@MODULE_NAME@", package.seeall)

require("parameters")
require("uart_test")

CFG_strTestName = "@TEST_NAME@"

CFG_aParameterDefinitions = {
	{
		name="verbose",
		default=0,
		help="Verbose level.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="unit",
		default=0,
		help="The index of the UART unit to test.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="mmio_rxd",
		default=nil,
		help="The index of the MMIO pin for the UART RXD signal.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="mmio_txd",
		default=nil,
		help="The index of the MMIO pin for the UART TXD signal.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="mmio_cts",
		default=nil,
		help="The index of the MMIO pin for the UART CTS signal.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="mmio_rts",
		default=nil,
		help="The index of the MMIO pin for the UART RTS signal.",
		mandatory=false,
		validate=parameters.test_uint32,
		constrains=nil
	},
	{
		name="flags",
		default="USE_RTS_CTS",
		help="The flags for the test.",
		mandatory=false,
		validate=parameters.test_choice_multiple,
		constrains="USE_RTS_CTS,HAS_OUTPUT_DRIVER,RECEIVE_OWN_ECHO"
	}
}


local atDefaultParameters = {
	[romloader.ROMLOADER_CHIPTYP_NETX500] = {
		mmio_rxd = 0xff,
		mmio_txd = 0xff,
		mmio_cts = 0xff,
		mmio_rts = 0xff
	},
	[romloader.ROMLOADER_CHIPTYP_NETX100] = {
		mmio_rxd = 0xff,
		mmio_txd = 0xff,
		mmio_cts = 0xff,
		mmio_rts = 0xff
	},
	[romloader.ROMLOADER_CHIPTYP_NETX56] = {
		mmio_rxd = 34,
		mmio_txd = 35,
		mmio_cts = 32,
		mmio_rts = 33
	},
	[romloader.ROMLOADER_CHIPTYP_NETX56B] = {
		mmio_rxd = 34,
		mmio_txd = 35,
		mmio_cts = 32,
		mmio_rts = 33
	},
	[romloader.ROMLOADER_CHIPTYP_NETX50] = {
		mmio_rxd = 34,
		mmio_txd = 35,
		mmio_cts = 0xff,
		mmio_rts = 0xff
	},
	[romloader.ROMLOADER_CHIPTYP_NETX10] = {
		mmio_rxd = 20,
		mmio_txd = 21,
		mmio_cts = 0xff,
		mmio_rts = 0xff
	}
}


function run(aParameters)
	----------------------------------------------------------------------
	--
	-- Parse the parameters and collect all options.
	--
	local ulVerboseLevel = tonumber(aParameters["verbose"])
	local uiUnit         = tonumber(aParameters["unit"])
	local ucMmioRxd      = tonumber(aParameters["mmio_rxd"])
	local ucMmioTxd      = tonumber(aParameters["mmio_txd"])
	local ucMmioCts      = tonumber(aParameters["mmio_cts"])
	local ucMmioRts      = tonumber(aParameters["mmio_rts"])
	
	-- Parse the test list.
	local astrElements = parameters.split_string(aParameters["flags"])
	local ulFlags = 0
	for iCnt,strElement in ipairs(astrElements) do
		if strElement=="USE_RTS_CTS" then
			ulFlags = ulFlags + uart_test.UARTTEST_FLAGS_Use_CTS_RTS
		elseif strElement=="HAS_OUTPUT_DRIVER" then
			ulFlags = ulFlags + uart_test.UARTTEST_FLAGS_Has_Output_Driver
		elseif strElement=="RECEIVE_OWN_ECHO" then
			ulFlags = ulFlags + uart_test.UARTTEST_FLAGS_Receive_Own_Echo
		else
			error(string.format("Unknown flag: %s", strElement))
		end
	end
	
	----------------------------------------------------------------------
	--
	-- Open the connection to the netX.
	-- (or re-use an existing connection.)
	--
	local tPlugin = tester.getCommonPlugin()
	if tPlugin==nil then
		error("No plug-in selected, nothing to do!")
	end
	
	-- Set default values.
	local tAsicTyp = tPlugin:GetChiptyp()
	if ucMmioRxd==nil or ucMmioTxd==nil or ucMmioCts==nil or ucMmioRts==nil then
		atDefaults = atDefaultParameters[tAsicTyp]
		if atDefaults==nil then
			error(string.format("No default values for chiptype %d!", tAsicTyp))
		end
		if ucMmioRxd==nil then
			ucMmioRxd = atDefaults.mmio_rxd
		end
		if ucMmioTxd==nil then
			ucMmioTxd = atDefaults.mmio_txd
		end
		if ucMmioCts==nil then
			ucMmioCts = atDefaults.mmio_cts
		end
		if ucMmioRts==nil then
			ucMmioRts = atDefaults.mmio_rts
		end
	end
	
	local ulResult = uart_test.run(tPlugin, "netx/uarttest_netx%d.bin", ulVerboseLevel, uiUnit, ucMmioRxd, ucMmioTxd, ucMmioCts, ucMmioRts, ulFlags)
	if ulResult~=0 then
		error("The UART test failed!")
	end
	
	print("")
	print(" #######  ##    ## ")
	print("##     ## ##   ##  ")
	print("##     ## ##  ##   ")
	print("##     ## #####    ")
	print("##     ## ##  ##   ")
	print("##     ## ##   ##  ")
	print(" #######  ##    ## ")
	print("")
end

