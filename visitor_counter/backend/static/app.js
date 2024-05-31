"use strict"

var query = document.querySelector.bind(document)

var app = {
	username: null,
	account_page: HTMLElement.prototype, // Для подсказок
	front_page: HTMLElement.prototype,
	sign_up_page: HTMLElement.prototype,
	sign_up_success_page: HTMLElement.prototype,
	add_device_page: HTMLElement.prototype,
	device_page: HTMLElement.prototype,
	device_remove_page: HTMLElement.prototype,
	device_page_state: {
		device_id: ""
	}
}

function hide_all() {
	app.front_page.classList.add("hide")
	app.account_page.classList.add("hide")
	app.sign_up_page.classList.add("hide")
	app.sign_up_success_page.classList.add("hide")
	app.add_device_page.classList.add("hide")
	app.device_page.classList.add("hide")
	app.device_remove_page.classList.add("hide")
}

function show_account_page() {
	var username = query(".account .username")

	username.innerText = app.username

	hide_all()
	app.account_page.classList.remove("hide")

	refresh_device_list()
}

function show_front_page() {
	hide_all()
	app.front_page.classList.remove("hide")
}

function show_front_page() {
	hide_all()
	app.front_page.classList.remove("hide")
}

function show_sign_up_page() {
	hide_all()
	app.sign_up_page.classList.remove("hide")
}

function show_sign_up_success_page() {
	hide_all()
	app.sign_up_success_page.classList.remove("hide")
}

function show_add_device_page() {
	hide_all()
	app.add_device_page.classList.remove("hide")
}

function show_device_page(device_id) {
	app.device_page_state.device_id = device_id

	hide_all()
	init_device_page()
	app.device_page.classList.remove("hide")
}

function show_device_remove_page() {
	hide_all()
	init_device_remove_page(app.device_page_state.device_id)
	app.device_remove_page.classList.remove("hide")
}

function app_init() {
	app.front_page = query(".front_page")
	app.account_page = query(".account")
	app.sign_up_page = query(".sign_up")
	app.sign_up_success_page = query(".sign_up_success")
	app.add_device_page = query(".add_device")
	app.device_page = query(".device")
	app.device_remove_page = query(".device_remove")

	fetch_user_account()
}
