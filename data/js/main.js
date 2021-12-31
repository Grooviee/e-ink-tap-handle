$(document).ready(function () {
  $("form").on("submit", function (e) {
    e.preventDefault();
  });
});

function onSubmit() {
  var useimage = $('input[name="useimage"]:checked').val();

  console.log(useimage == "false");
  console.log(useimage == false);
  if (useimage == "false") {
    var formValue = $("form").serializeArray();
    console.log(formValue);
    $.ajax({
      url: "/data",
      type: "POST",
      cache: false,
      data: formValue,
      processData: false,
      contentType: false,
      beforeSend: function () {
        console.log("upload start");
      },
      success: function (data) {
        console.log("success save");
      },
      error: function (error) {
        console.log(error);
      },
    });
    alert("data saved");
  } else {
    var formData = new FormData();

    var image_file = document.getElementById("image_input");
    var file = image_file.files[0];
    if (file == null) {
      $("#image_error_text").html("Image file is required").show();
      return false;
    } else if (/bmp/.test(file.type)) {
      $("#image_error_text").hide();
    } else {
      $("#image_error_text").html("Only support image format BMP").show();
      return false;
    }
    if (file.size >= 500 * 1024) {
      $("#image_error_text").html("Image size does not exceed 500Kb").show();
      return false;
    }

    formData.append("data", file);

    $.ajax({
      url: "/",
      type: "POST",
      cache: false,
      data: formData,
      processData: false,
      contentType: false,
      dataType: file.type,
      beforeSend: function () {
        console.log("upload start");
      },
      success: function (data) {
        console.log("success upload");
      },
    });
    alert("image uploaded");
  }
}

function showPicture(obj) {
  var reg_info_file = document.getElementById("image_input");
  var file = reg_info_file.files[0];

  var f = obj.files[0];
  if (/bmp/.test(f.type)) {
    $("#image_error_text").hide();
  } else {
    $("#image_error_text").html("Only support image format BMP").show();
    return false;
  }
  if (f.size >= 500 * 1024) {
    $("#image_error_text").html("Image size does not exceed 500Kb").show();
    return false;
  }

  var baseID = "#image_base64";
  var hideID = "#image_w";
  var fr = new FileReader();
  fr.readAsDataURL(f);

  $(baseID).removeAttr("width");
  $(baseID).removeAttr("height");
  fr.onloadend = function (e) {
    $(hideID).hide();
    $(baseID).show();
    $(baseID).attr("src", e.target.result);
    $(baseID)[0].onload = function () {
      var upimgw = $(baseID).width();
      var upimgh = $(baseID).height();
      $(baseID).attr("width", imgPercentScale(122, 255, upimgw, upimgh).w);
      $(baseID).attr("height", imgPercentScale(122, 255, upimgw, upimgh).h);
    };
  };
}

function imgPercentScale(boxw, boxh, imgw, imgh) {
  var res = {};
  var wper = imgw / boxw;
  var hper = imgh / boxh;
  if (wper <= 1 && hper < 1) {
    res.w = imgw;
    res.h = imgh;
    return res;
  }
  if (wper > 1 && hper < 1) {
    res.w = boxw;
    var rhper = boxw / imgw;
    res.h = imgh * rhper;
    return res;
  }
  if (wper <= 1 && hper > 1) {
    res.h = boxh;
    var rwper = boxh / imgh;
    res.w = imgw * rwper;
    return res;
  }
  if (wper > 1 && hper > 1) {
    if (wper >= hper) {
      res.w = boxw;
      var rhper = boxw / imgw;
      res.h = imgh * rhper;
      return res;
    } else {
      res.h = boxh;
      var rwper = boxh / imgh;
      res.w = imgw * rwper;
      return res;
    }
  }
}
