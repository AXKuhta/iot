
function ago(time) {
	var formatter = new Intl.RelativeTimeFormat(undefined, {
		style: "short"
	})

	var delta = new Date() - time
	var unit = "seconds"

	delta /= 1000

	if (delta >= 60) {
		delta /= 60
		unit = "minutes"
	}

	if (delta >= 60) {
		delta /= 60
		unit = "hours"
	}

	if (delta >= 24) {
		delta /= 24
		unit = "days"
	}

	return formatter.format(-Math.floor(delta), unit)
}

var timefmt = new Intl.DateTimeFormat(undefined, {
	dateStyle: "medium",
	timeStyle: "short",
})