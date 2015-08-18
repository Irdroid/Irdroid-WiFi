-- Copyright 2015 Georgi Bakalski
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.wsocket", package.seeall)

function index()
	if not nixio.fs.access("/etc/serwebproxy.cfg") then
		return
	end
	
	local page

	page = entry({"admin", "services", "wsocket"}, cbi("wsocket/wsocket"), _("Websocket to Serial Gateway"), 50)
	page.dependent = true

	page = entry({"mini", "services", "wsocket"}, cbi("wsocket/wsocket", {autoapply=true}), _("Websocket to Serial Gateway"), 50)
	page.dependent = true
end
