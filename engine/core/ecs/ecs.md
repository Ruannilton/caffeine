Types:
    - Component: uint64
    - Archtype: Component[]
    - Entity: uint64
    - Storage: void*[][]

Actions:
- Cadastrar Componente:
    - Nome do componente
    - Tamanho do componente
    - Alinhamento do componente
    - Nome deve ser único

- Excluir componente:
    - Excluir archtypes que dependem do componente

- Listar archtypes associadas ao componente

- Cadastrar Archtype:
    - Associar archtype aos componentes que formam ela
    - Cria seu respectivo storage

- Excluir Archtype:
    - Desassocia archtype dos componentes que formam ela
    - Apaga os dados das entidades armazenados na storage

- Modificar Archtype:
    - Adiciona/remove a coluna dos componente que foi alterado na respectiva storage
        - Caso o archetype resultante coincidir com um existente, move as entidades para a estorage existente

- Instanciar Entidade:
    - Aloca ultima linha no storage respectivo storage

- Remover Entidade
    - Transfere a ultima linha para a linha a ser removida

- Adicionar/Remover componente da Entidade
    - Move os dados da entidade para o storage do archtype resultante
    