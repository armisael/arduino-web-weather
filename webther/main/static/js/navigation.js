$(document).ready(function() {
  "use strict";

//  $('ul.nav-pills-magic li.active').click(function() {
//    console.log($(this));
//    var $this = $(this)
//      ;
//    $this.parent().find('li:not(.active)').show();
//  })
  $(document).click(function(e) {
    // avoid hiding when clicking on a .make-popover
    if ($(e.target).hasClass('make-popover')) return;

    // hide if we clicked on anything but a .popover-inner
    if (!$(e.target).closest('.popover-inner').length) {
      $('.make-popover.showing').popover('hide');
    };
  });

  $('.make-popover').popover({
    'placement': 'bottom',
    'trigger': 'manual',
    'content': function() {
      return $(this).find('.content').html();
    }
  }).click(function() {
    $('.make-popover.showing').not(this).popover('hide');
    $(this).popover('toggle').addClass('showing');
  });

});
