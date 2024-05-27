


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
