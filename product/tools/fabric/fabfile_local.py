'''
Environments dictionary is used to override the fabric.env
dictionary using the e method. See fabfile.py for details.
'''
environments = {
        "local": {
            "hosts": ["localhost"],
            },
        "staging": {
            "hosts": [
                "escuser@esc-game-server.local",
                ],
            },
	    "production": {
            "hosts": [""],
            },
        }		