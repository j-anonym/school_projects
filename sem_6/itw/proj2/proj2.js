function showMenuItems() {
  var navigation = document.getElementById("nav-menu");
  if (navigation.style.display === "block") {
    navigation.style.display = "none";
  } else {
    navigation.style.display = "block";
  }
}

function checkWindowSize() {
  var x = document.getElementById("nav-menu");
  if (window.innerWidth >= 700) {
      x.style.display = "block";
  } else {
      x.style.display = "none";
  }

}

window.addEventListener('resize', checkWindowSize);

function openInterest(evt, interest) {
  var i, tabcontent, tablinks;

  tabcontent = document.getElementsByClassName("tab-content");
  for (i = 0; i < tabcontent.length; i++) {
    tabcontent[i].style.display = "none";
  }

  tablinks = document.getElementsByClassName("tab-links");
  for (i = 0; i < tablinks.length; i++) {
    tablinks[i].className = tablinks[i].className.replace(" active", "");
  }

  document.getElementById(interest).style.display = "block";
  evt.currentTarget.className += " active";
}