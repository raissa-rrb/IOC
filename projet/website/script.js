/* recup button */
let button_on = document.getElementById("button_on");
let button_off = document.getElementById("button_off");

/* recup photoR value */
let balisePhotoR = document.getElementById("photoVal");

/* écouter event */
button_on.addEventListener("click", ()=> {
    console.log("led on");
    balisePhotoR.innerHTML = "aaah % ";
});

button_off.addEventListener("click", function() {
    console.log("led off");
    balisePhotoR.innerHTML = "oooh % ";

});

