function loadShader(targetFile){
    var url = "shader/" + targetFile;
    var req = new XMLHttpRequest();
    req.open("GET", url, false);
    req.send(null);

    if(req.status == 200){
        return req.responseText;
    }
    else{
        console.warn("Warning when loading " + targetFile + " No Such File Exist!")
        return null;
    }
}