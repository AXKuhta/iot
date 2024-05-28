


async function fetch_user_account() {
	var request = await fetch("/users/me")

	if (request.status == 200) {
		var result = await request.json()

		app.username = result.username
		show_account_page()
	} else {
		show_front_page()
	}
}

async function handle_sign_in(form) {
	var fd = new FormData(form)

	form["status"].value = "Please wait..."
	form["status"].style.color = "inherit"

	try {
		var request = await fetch("/login", {
			method: "POST",
			body: fd
		})
	} catch (e) {
		form["status"].value = "Network error"
		form["status"].style.color = "red"
		return
	}

	if (request.status == 200) {
		form["status"].value = ""
		fetch_user_account()
		form.reset()
	} else {
		var response = await request.json()
		form["status"].value = "Error: " + response.detail
		form["status"].style.color = "red"
	}
}

async function handle_sign_out(form) {
	form["status"].value = "Please wait..."
	form["status"].style.color = "inherit"

	try {
		var request = await fetch("/logout", {
			method: "POST"
		})
	} catch (e) {
		form["status"].value = "Network error"
		form["status"].style.color = "red"
		return
	}

	if (request.status == 200) {
		form["status"].value = ""
		fetch_user_account()
	} else {
		var response = await request.json()
		form["status"].value = "Error: " + response.detail
		form["status"].style.color = "red"
	}
}

async function request_account_creation(form) {
	var fd = new FormData(form)

	var username = fd.get("username")
	var password = fd.get("password")

	form["status"].value = "Please wait..."
	form["status"].style.color = "inherit"

	try {
		var request = await fetch("/register", {
			method: "POST",
			body: JSON.stringify({ username, password }),
			headers: {
				"Content-Type": "application/json"
			},
		})
	} catch (e) {
		form["status"].value = "Network error"
		form["status"].style.color = "red"
		return
	}

	if (request.status == 200) {
		form["status"].value = ""
		show_sign_up_success_page()
		form.reset()
	} else {
		var response = await request.json()
		form["status"].value = "Error: " + response.detail
		form["status"].style.color = "red"
	}
}
