$(document).ready(function() {
  "use strict";

  $('ul.nav-pills-magic li.active').click(function() {
    console.log($(this));
    var $this = $(this)
      ;
    $this.parent().find('li:not(.active)').show();
  })
});
