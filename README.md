<div>
<img src="https://user-images.githubusercontent.com/942052/220032555-99cd216e-fd40-4dcf-9794-4eb6c11b642e.png"  alt="nodable-logo.png"/>
</div>

<div>
<a href="https://github.com/berdal84/Nodable/actions?query=workflow%3Aci" title="linux">
<img src="https://github.com/berdal84/nodable/workflows/ci/badge.svg"  alt="badge-linux.svg"/>
</a>
</div>

# Nodable is node-able !

## Introduction:

The goal of *Nodable* is:
- to **provide an original hybrid source code editor**, using both textual and nodal paradigms.
- to **learn software development**, by learning language related stuff (parsers, AST, compilation, etc.)

The goal of *Nodable* is **not**:
- To handle more than 1 language
- To be a production tool.

In Nodable, the textual and nodal points of view are strongly linked, in both ways:

- A change to the source code will update the graph.
- A change to the graph will update* the source code.

_* Nodable persistent state is the source code. Some minor changes on the graph, like changing their position or adding orphaned nodes, might not update the source code_


### Screenshots

<img width="500" src="https://user-images.githubusercontent.com/942052/161857692-97786562-c30c-470c-9e07-62b240a4a222.gif"/>
<img width="500" src="https://user-images.githubusercontent.com/942052/161857699-eedb1c42-2b49-4bea-8da7-20f1b522cf73.gif"/>
<img width="500" src="https://user-images.githubusercontent.com/942052/211735062-21d29b63-77da-4100-8738-61805ad11318.png"/>

## Download

Follow the instruction from the [latest release](https://github.com/berdal84/Nodable/releases/latest) section.

In case you prefer to build *Nodable* from sources, read [HOW-TO-BUILD.md](./HOW-TO-BUILD.md).

## License

**Nodable** is licensed under the GPL License, see [LICENSE](./LICENSE). Each submodule is licensed, browse [./libs](./libs).

## Credits

**Nodable** is developed by [@berdal84](https://github.com/berdal84) and rely on many libraries ([more info](./libs/README.md)).
More information about this project on [Nodable website](https://nodable.42borgata.com/).


