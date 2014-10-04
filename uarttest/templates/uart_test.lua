module("uart_test", package.seeall)

-----------------------------------------------------------------------------
--   Copyright (C) 2014 by Christoph Thelen                                --
--   doc_bacardi@users.sourceforge.net                                     --
--                                                                         --
--   This program is free software; you can redistribute it and/or modify  --
--   it under the terms of the GNU General Public License as published by  --
--   the Free Software Foundation; either version 2 of the License, or     --
--   (at your option) any later version.                                   --
--                                                                         --
--   This program is distributed in the hope that it will be useful,       --
--   but WITHOUT ANY WARRANTY; without even the implied warranty of        --
--   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         --
--   GNU General Public License for more details.                          --
--                                                                         --
--   You should have received a copy of the GNU General Public License     --
--   along with this program; if not, write to the                         --
--   Free Software Foundation, Inc.,                                       --
--   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             --
-----------------------------------------------------------------------------


-- TODO: document the functions with doxygen. See here for details: ftp://ftp.tex.ac.uk/ctan%3A/web/lua2dox/lua2dox-refm.pdf

require("bit")
require("muhkuh")
require("romloader")


-----------------------------------------------------------------------------
--                           Definitions                                   --
-----------------------------------------------------------------------------

UARTTEST_VERSION_MAJ  = ${VERSION_MAJ}
UARTTEST_VERSION_MIN  = ${VERSION_MIN}
UARTTEST_VERSION_VCS  = ${VERSION_VCS}

UARTTEST_FLAGS_Use_CTS_RTS       = ${UARTTEST_FLAGS_Use_CTS_RTS}
UARTTEST_FLAGS_Has_Output_Driver = ${UARTTEST_FLAGS_Has_Output_Driver}
UARTTEST_FLAGS_Receive_Own_Echo  = ${UARTTEST_FLAGS_Receive_Own_Echo}


HEADER_MAGIC          = 0x686f6f6d


-----------------------------------------------------------------------------
--
-- Default callback functions.
--
local ulProgressLastTime    = 0
local fProgressLastPercent  = 0
local ulProgressLastMax     = nil
PROGRESS_STEP_PERCENT       = 10

function default_callback_progress(ulCnt, ulMax)
	local fPercent = ulCnt * 100 / ulMax
	local ulTime = os.time()
	if ulProgressLastMax ~= ulMax
	   or ulCnt == 0
	   or ulCnt == ulMax
	   or fProgressLastPercent - fPercent > PROGRESS_STEP_PERCENT
	   or ulTime - ulProgressLastTime > 1 then
		fProgressLastPercent = fPercent
		ulProgressLastMax = ulMax
		ulProgressLastTime = ulTime
		print(string.format("%d%% (%d/%d)", fPercent, ulCnt, ulMax))
	end
	return true
end


function default_callback_message(a,b)
	if type(a)=="string" then
		io.write("[netx] ")
		local strLastChar = string.sub(a, -1)
		if strLastChar == "\r" or strLastChar == "\n" then
			io.write(a)
		else
			print(a)
		end
	end
	return true
end


-----------------------------------------------------------------------------


local function get_dword(strData, ulOffset)
	return string.byte(strData,ulOffset) + string.byte(strData,ulOffset+1)*0x00000100 + string.byte(strData,ulOffset+2)*0x00010000 + string.byte(strData,ulOffset+3)*0x01000000
end


-----------------------------------------------------------------------------


function run(tPlugin, strPattern, ulVerboseLevel, uiUnit, ucMmioRxd, ucMmioTxd, ucMmioCts, ucMmioRts, ulFlags, fnCallbackProgress, fnCallbackMessage)
	-- Set the defaults for the optional parameter.
	fnCallbackProgress = fnCallbackProgress or default_callback_progress  -- Use the built-in callback.
	fnCallbackMessage = fnCallbackMessage or default_callback_message     -- Use the built-in callback.

	-- Get the chip type.
	local tAsicTyp = tPlugin:GetChiptyp()
	local uiAsic


	-- Get the binary for the ASIC.
	if tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX50 then
		uiAsic = 50
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX100 or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX500 then
		uiAsic = 500
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX10 then
		uiAsic = 10
	elseif tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX56 or tAsicTyp==romloader.ROMLOADER_CHIPTYP_NETX56B then
		uiAsic = 56
	else
		error("Unknown chiptyp!")
	end

	strBinaryName = string.format(strPattern, uiAsic)

	-- Try to load the binary.
	strData, strMsg = muhkuh.load(strBinaryName)
	if not strData then
		error(string.format("Failed to load binary '%s': ", strBinaryName, strMsg))
	end

	-- Check the magic cookie.
	ulMagicCookie = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_acMagic} + 1)
	if ulMagicCookie~=HEADER_MAGIC then
		error("The binary has no magic at the beginning. Is this an old binary?")
	end

	-- Check the version.
	ulVersionMaj = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_ulVersionMaj} + 1)
	ulVersionMin = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_ulVersionMin} + 1)
	strVersionVcs = string.sub(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_acVersionVcs} + 1, ${OFFSETOF_VERSION_HEADER_STRUCT_acVersionVcs} + 16)
	while string.byte(strVersionVcs,-1)==0 do
		strVersionVcs = string.sub(strVersionVcs,1,-2)
	end
	print(string.format("Binary version: %d.%d.%s", ulVersionMaj, ulVersionMin, strVersionVcs))
	print(string.format("Script version: %d.%d.%s", UARTTEST_VERSION_MAJ, UARTTEST_VERSION_MIN, UARTTEST_VERSION_VCS))
	if ulVersionMaj~=UARTTEST_VERSION_MAJ then
		error("The major version number of the binary and this script differs. Cowardly refusing to continue.")
	end
	if ulVersionMin~=UARTTEST_VERSION_MIN then
		error("The minor version number of the binary and this script differs. Cowardly refusing to continue.")
	end
	if strVersionVcs~=UARTTEST_VERSION_VCS then
		error("The VCS version number of the binary and this script differs. Cowardly refusing to continue.")
	end

	-- Get the header from the binary.
	ulLoadAddress           = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_pulLoadAddress} + 1)
	ulExecutionAddress      = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_pfnExecutionAddress} + 1)
	ulParameterStartAddress = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_pulParameterStart} + 1)
	ulParameterEndAddress   = get_dword(strData, ${OFFSETOF_VERSION_HEADER_STRUCT_pulParameterEnd} + 1)

	-- Show the information:
	print(string.format("load address:       0x%08x", ulLoadAddress))
	print(string.format("start address:      0x%08x", ulExecutionAddress))
	print(string.format("buffer start:       0x%08x", ulParameterStartAddress))
	print(string.format("buffer end:         0x%08x", ulParameterEndAddress))

	print(string.format("downloading exe to 0x%08x", ulLoadAddress))
	tPlugin:write_image(ulLoadAddress, strData, fnCallbackProgress, strData:len())

	local ulMmioCfg = 0
	ulMmioCfg = ulMmioCfg + (ucMmioRxd * math.pow(256, ${UARTTEST_PARAMETER_MMIO_INDEX_RXD}))
	ulMmioCfg = ulMmioCfg + (ucMmioTxd * math.pow(256, ${UARTTEST_PARAMETER_MMIO_INDEX_TXD}))
	ulMmioCfg = ulMmioCfg + (ucMmioCts * math.pow(256, ${UARTTEST_PARAMETER_MMIO_INDEX_CTS}))
	ulMmioCfg = ulMmioCfg + (ucMmioRts * math.pow(256, ${UARTTEST_PARAMETER_MMIO_INDEX_RTS}))
	
	
	-- Transfer the data with 115200 baud.
	local ulBaud = 115200
	
	-- All UART units are clocked with 100MHz.
	local ulUnitFrequency = 100000000
	
	-- Calculate the baud rate divider from the requested speed and the unit clocking.
	local ulBaudRateDivider = math.floor(((ulBaud*16*65536)/ulUnitFrequency)+0.5)
	if ulBaudRateDivider==0 then
		error("The baud rate divider is 0!")
	elseif ulBaudRateDivider>0xffff then
		error("The baud rate divider exceeds the allowed range!")
	end
	
	-- Collect the parameter.
	local atParameters = {
		ulVerboseLevel,                      -- Verbose level. 
		uiUnit,                              -- The UART unit number.
		ulBaudRateDivider,                   -- The baud rate divider.
		ulMmioCfg,                           -- The MMIO configuration.
		ulFlags                              -- The flags for the test.
	}
	
	-- Write the standard header.
	tPlugin:write_data32(ulParameterStartAddress+0x00, 0xFFFFFFFF)                    -- Init the test result.
	tPlugin:write_data32(ulParameterStartAddress+0x04, ulParameterStartAddress+0x0c)  -- Address of test parameters.
	tPlugin:write_data32(ulParameterStartAddress+0x08, 0x00000000)                    -- Reserved
	
	-- Write the extended header.
	for iIdx,ulValue in ipairs(atParameters) do
		tPlugin:write_data32(ulParameterStartAddress+0x0c+((iIdx-1)*4), ulValue)
	end

	-- Call the netX program.
	print("__/Output/____________________________________________________________________")
	tPlugin:call(ulExecutionAddress, ulParameterStartAddress, fnCallbackMessage, 0)
	print("")
	print("______________________________________________________________________________")
	
	-- Get the result.
	local ulResult = tPlugin:read_data32(ulParameterStartAddress)
	return ulResult
end


