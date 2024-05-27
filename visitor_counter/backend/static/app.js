"use strict"

var query = document.querySelector.bind(document)

var app = {}

function hide_all() {
	app.front_page.classList.add("hide")
	app.account_page.classList.add("hide")
}

function show_account_page() {
	var username = query(".account .username")

	username.innerText = app.username

	hide_all()
	app.account_page.classList.remove("hide")
}

function show_front_page() {
	hide_all()
	app.front_page.classList.remove("hide")
}

function app_init() {
	app = {
		username: null,
		account_page: query(".account"),
		front_page: query(".front_page")
	}

	fetch_user_account();
}
