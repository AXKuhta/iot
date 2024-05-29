
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

async function refresh_device_list() {
	var device_list = app.account_page.querySelector(".device-list")
	var template = device_list.children[0]

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

			userlabel.innerText = device.userlabel
			lsh.innerText = ago(new Date(device.last_seen_healthy))
			device_id.innerText = device.device_id

			list_entry.classList.remove("hide")

			new_children.push(list_entry)
		}

		device_list.replaceChildren(...new_children)

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
