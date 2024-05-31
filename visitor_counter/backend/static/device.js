
async function request_device_claim(form) {
	var fd = new FormData(form)

	form["status"].value = "Please wait..."
	form["status"].style.color = "inherit"

	try {
		var request = await fetch("/add_device", {
			method: "POST",
			body: fd
		})
	} catch (e) {
		form["status"].value = "Network error"
		form["status"].style.color = "red"
		return
	}

	if (request.status == 200) {
		form["status"].value = "OK"

		form.reset()
	} else {
		var response = await request.json()
		form["status"].value = "Error: " + response.detail
		form["status"].style.color = "red"
	}
}

async function fetch_visitors_from(device_id, from_utc_ts) {
	try {
		var request = await fetch("/devices/" + device_id + "/visitors", {
			method: "POST",
			body: JSON.stringify({ from_utc_ts: from_utc_ts }),
			headers: {
				"Content-Type": "application/json"
			},
		})
	} catch (e) {
		return { today: "?", total: "?" }
	}

	if (request.status == 200) {
		return await request.json()
	} else {
		var response = await request.json()
		console.log(response)
		return { today: "?", total: "?" }
	}
}

async function refresh_device_list() {
	var device_list = app.account_page.querySelector(".device-list")
	var template = device_list.children[0]

	var start_of_day = (new Date()).setHours(0, 0, 0, 0)
	var utc_start_of_day = start_of_day + (new Date()).getTimezoneOffset()*60*1000
	var utc_epoch_start_of_day = utc_start_of_day / 1000 >> 0

	try {
		var request = await fetch("/list_devices", {
			method: "POST"
		})
	} catch (e) {
		return
	}

	if (request.status == 200) {
		var list = await request.json()

		var new_children = [template]

		for (var device of list) {
			var list_entry = template.cloneNode(true)
			var lsh = list_entry.querySelector(".lsh")
			var userlabel = list_entry.querySelector(".userlabel")
			var device_id = list_entry.querySelector(".device_id")
			var visitors_total = list_entry.querySelector(".visitors-total")
			var visitors_today = list_entry.querySelector(".visitors-today")

			var visitors = await fetch_visitors_from(device.device_id, utc_epoch_start_of_day)

			visitors_total.innerText = visitors.total
			visitors_today.innerText = visitors.today

			userlabel.innerText = device.userlabel
			lsh.innerText = ago(new Date(device.last_seen_healthy))
			device_id.innerText = device.device_id

			list_entry.classList.remove("hide")

			new_children.push(list_entry)
		}

		device_list.replaceChildren(...new_children)

	} else if (request.status == 401) {
		return fetch_user_account()
	} else {
		var response = await request.json()
		console.log(response)
	}
	  

	if (app.account_page.classList.contains("hide")) {
		// Stop
	} else {
		setTimeout(refresh_device_list, 5000)
	}
}

async function init_device_page() {
	var device_id_box = app.device_page.querySelector(".device_id")
	var userlabel_box = app.device_page.querySelector(".userlabel")
	var settings_form = app.device_page.querySelector("form")

	device_id_box.innerText = app.device_page_state.device_id

	try {
		var request = await fetch("/devices/" + app.device_page_state.device_id, {
			method: "GET"
		})
	} catch (e) {
		return
	}

	if (request.status == 200) {
		var info = await request.json()

		userlabel_box.innerText = info.userlabel
		settings_form["userlabel"].value = info.userlabel
		settings_form["entrance"].value = info.direction_of_entrance
		settings_form["status"].value = ""
	} else {
		var response = await request.json()
		console.log(response)
	}

	refresh_device_page()
}

async function refresh_device_page() {
	var time = app.device_page.querySelector(".time")
	var u = app.device_page.querySelector(".u")
	var d = app.device_page.querySelector(".d")
	var l = app.device_page.querySelector(".l")
	var r = app.device_page.querySelector(".r")

	try {
		var request = await fetch("/devices/" + app.device_page_state.device_id, {
			method: "GET"
		})
	} catch (e) {
		return
	}

	if (request.status == 200) {
		var info = await request.json()

		time.innerText = timefmt.format(new Date(info.data.time))
		u.innerText = +info.data.u
		d.innerText = +info.data.d
		l.innerText = +info.data.l
		r.innerText = +info.data.r
	} else if (request.status == 401) {
		return fetch_user_account()
	} else {
		var response = await request.json()
		console.log(response)
	}

	if (app.device_page.classList.contains("hide")) {
		// Stop
	} else {
		setTimeout(refresh_device_page, 5000)
	}
}

async function init_device_remove_page(device_id) {
	var device_id_box = app.device_remove_page.querySelector(".device_id")
	var userlabel_box = app.device_remove_page.querySelector(".userlabel")

	device_id_box.innerText = device_id

	try {
		var request = await fetch("/devices/" + device_id, {
			method: "GET"
		})
	} catch (e) {
		return
	}

	if (request.status == 200) {
		var info = await request.json()

		userlabel_box.innerText = info.userlabel
	} else {
		var response = await request.json()
		console.log(response)
	}
}

async function remove_device(device_id) {
	try {
		var request = await fetch("/devices/" + device_id + "/remove", {
			method: "GET"
		})
	} catch (e) {
		return
	}

	if (request.status == 200) {
		fetch_user_account()
	} else {
		var response = await request.json()
		console.log(response)
	}
}

async function save_device_settings(form) {
	var fd = new FormData(form)

	form["status"].value = "Please wait..."
	form["status"].style.color = "inherit"

	try {
		var request = await fetch("/devices/" + app.device_page_state.device_id + "/settings", {
			method: "POST",
			body: fd
		})
	} catch (e) {
		form["status"].value = "Network error"
		form["status"].style.color = "red"
		return
	}

	if (request.status == 200) {
		form["status"].style.color = "green"
		form["status"].value = "Settings saved!"

		app.device_page.querySelector(".userlabel").innerText = fd.get("userlabel")
	} else {
		var response = await request.json()
		form["status"].value = "Error: " + response.detail
		form["status"].style.color = "red"
	}
}