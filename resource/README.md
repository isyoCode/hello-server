# Static Resources

This directory contains static files served by YoyoCppServer.

## Directory Structure

```
resource/
├── css/           # CSS stylesheets
│   └── style.css
├── images/        # Image files
│   ├── 1.png
│   ├── 3.jpg
│   ├── scenery1.jpg
│   ├── scenery2.jpg
│   ├── scenery3.jpg
│   ├── scenery4.jpg
│   ├── background.jpg
│   └── someone.jpg
├── *.html         # HTML pages
│   ├── index.html
│   ├── login.html
│   ├── register.html
│   ├── dashboard.html
│   ├── game1.html
│   ├── game2.html
│   └── test_images.html
└── README.md      # This file
```

## Usage

The server automatically serves files from this directory. The path is computed relative to the executable location, ensuring portability across different systems.

## Adding New Resources

1. Place your files in the appropriate subdirectory
2. Add corresponding routes in `examples/basic_server.cc` if needed
3. Restart the server

**Note**: This directory is empty by default. Copy your static resources here before running the server.
