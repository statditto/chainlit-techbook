
= Chainlitの基本機能を概観する

//lead{
シンプルなウェブアプリケーションと豊富な図を用いて、Chainlitの機能を解説します。
//}

== 本章で利用するアプリケーション

本章では、ルールベースで応答するシンプルなウェブアプリケーション @<fn>{support} を例に、Chainlitの機能を説明します。アプリケーションの構成は @<img>{app_overview} の通りです。ユーザーはブラウザからアクセスしてチャットを行います。チャットの内容は PostgreSQL データベースに保存されます。

//footnote[support][本章のソースコードは以下から参照できます。@<href>{https://github.com/kimajin/chainlit-playground}]

//image[app_overview][アプリケーション構成図][scale=0.6]{
//}

== ログイン

ユーザーが Chainlit アプリケーションにアクセスすると、ログイン画面が表示されます（@<img>{login}）。
今回は、@<code>{@cl.password_auth_callback} デコレーターを修飾したコールバック関数を定義して、パスワード認証を行っています。

//image[login][ログイン画面][scale=0.6]{
//}

特に、認証機能を有効にすることで、以下の機能が利用できるようになります。

 * チャット履歴の永続化（ @<code>{@cl.data_layer} ）
 * チャットの再開（ @<code>{@cl.on_chat_resume} ）

Chainlitはその他の認証方法もサポートしています。詳細は Chainlit のドキュメントを参照してください。

 * @<href>{https://docs.chainlit.io/authentication/overview}

認証に成功したら @<code>{cl.User} オブジェクトが返され、チャット開始画面へと遷移します（認証に失敗したら @<code>{None} を返します）。

== チャット

//image[chat_start][チャット開始画面][scale=0.6]{
//}

ログイン後のチャット開始画面は @<img>{chat_start} のようになっています。

この画面には、以下のような UI 要素があります。

 * アシスタント選択（画像上部）
 * スターター（画面中央下部）
 * 過去のチャット（画像左）
 * 入力欄直下の歯車・猫のアイコン


このセクションでは、チャット画面のこれらのUI要素に関する、基本的な機能と実装方法について説明します。

=== スターター設定（ @<code>{@cl.set_starters} ）

スターターは、ユーザーがチャットを開始する際に選択できるプリセットのメッセージです。スターターをクリックすると対応するメッセージが送信され、チャットが開始されます。

スターターを設定するには、@<code>{@cl.set_starters} デコレーターを使用して、スターターのリストを定義します。個別のスターターは、@<code>{cl.Starter} クラスのインスタンスとして定義します。

//emlist[Starter の実装][python]{
@cl.set_starters
async def set_starters(user: cl.User | None) -> list[cl.Starter]:
    return [
        cl.Starter(
            label="Message",        # 表示されるラベル
            message="Hello World!", # 送信されるメッセージ
        ),
        ...
    ]
//}

スターターを利用すると、対応するメッセージが送信されてチャットが開始されます（@<img>{starter}）。
//image[starter][スターターの利用画面][scale=0.6]{
//}

スターターは、ユーザーがチャットを開始する際のガイドとして機能し、特に初めてのユーザーにとって便利です。

=== アシスタント選択（ @<code>{@cl.set_chat_profiles} ）

アシスタントを選択するUIによって、ChatGPTのモデル切り替えと同様の体験を提供できます。

//image[chat_profile][Chat Profileの選択][scale=0.6]{
//}


UIの提供は、@<code>{@cl.set_chat_profiles} と @<code>{cl.ChatProfile} を利用して実装します。

//emlist[Chat Profile の実装][python]{
@cl.set_chat_profile
async def set_chat_profiles() -> list[cl.ChatProfile]:
    return [
        cl.ChatProfile(
            name="Assistant Alice",
            markdown_description="A friendly assistant.",
        ),
        cl.ChatProfile(
            name="Assistant Bob",
            markdown_description="A helpful assistant.",
        ),
    ]
//}

選択したアシスタントは @<code>{cl.user_session.get("chat_profile")} で取得できます。

=== チャット再開（ @<code>{@cl.on_chat_resume} ）

@<img>{chat_start}の画面左には、ChatGPTと同様にユーザーの過去のチャット履歴が表示されており、チャットを再開できます。

チャット再開機能を実現するために必要な要素は次のとおりです。

 * 認証（ユーザー識別）
 * データレイヤー（履歴保存）
 * @<code>{@cl.on_chat_resume} の実装（復元処理）

最後の要素は、以下の数行で実装できます。

//emlist[Chat Resume の実装][python]{
@cl.on_chat_resume
async def on_chat_resume(_: ThreadDict) -> None:
    pass
//}

==== データレイヤー ( @<code>{@cl.data_layer} )

チャット再開には履歴保存が必要です。そのため、Chainlitはデータレイヤーという抽象化層を通してチャットデータの保存と取得を行います。

データレイヤーを利用するメリットは次のとおりです。

 * アプリケーション側はデータベースの種類を意識する必要がない
 * PostgreSQLや独自ストレージなど、任意の保存先を柔軟に使える

また、Chainlit は API 実装済みの @<code>{SQLAlchemyDataLayer} を提供しており、データベースとテーブルを用意するだけで利用できます。
作成するテーブルは以下のリンク先から確認できます。

 * @<href>{https://docs.chainlit.io/data-layers/sqlalchemy}

特に以下のテーブルにデータを保存します。@<fn>{note-on-data-layer}

 * @<code>{users}：ユーザー情報
 * @<code>{threads}：チャットスレッド
 * @<code>{steps}：チャット内のメッセージ

//footnote[note-on-data-layer][チャットやメッセージが更新・削除可能なため、対応するレコードもミュータブルになっています。特に、イベントをイミュータブルに追加していく構造とはなっていません。]

//image[data_layer][Data Layerの例。 steps テーブルにメッセージが保存されている。][scale=1.0]{
//}

公式で提供されている @<code>{SQLAlchemyDataLayer} を使うと、アプリ側の実装はほぼ設定だけで済みます。

//emlist[Data Layer の実装][python]{
@cl.data_layer
def data_layer() -> SQLAlchemyDataLayer:
    return SQLAlchemyDataLayer(conninfo=os.environ["CHAINLIT_CONNINFO"])
//}

主なポイントは次のとおりです。

 * @<code>{conninfo} にはデータベースの接続URIを与える
 * データベースとテーブルを用意すればすぐに利用できる

なお、@<code>{BaseDataLayer} を継承して独自データレイヤーを実装することも可能です。必要な API は以下で確認できます。

 * @<href>{https://docs.chainlit.io/api-reference/data-persistence/custom-data-layer}

=== コマンド設定（ @<code>{cl.context.emitter.set_commands} ）

メッセージ入力欄の下にあるボタンはコマンドと呼ばれる機能です。ボタンを押すか、@<img>{command_meow_from_input} のように「/」検索で利用できます。
//image[command_meow_from_input][Commandの利用画面][scale=0.6]{
//}

コマンドの実行フローは次のとおりです。

 * 入力欄のボタン、もしくは「/」検索でコマンドを選択する
 * メッセージを送信すると、選択したコマンドが @<code>{message.command} に設定される
 * @<code>{message.command} の値に応じて処理を振り分ける

//image[command_wc_output][Word Count コマンドの実行結果][scale=0.6]{
//}

選択されたコマンドは、メッセージの @<code>{command} 属性から判別でき、その文字列に基づいて処理を振り分けます。

//emlist[Command 処理の実装][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    if command := message.command:
        match command:
            case "Meow":
                await cl.Message(content="Meow!").send()
            case "Word Count":
                word_count = len(message.content.split())
                await cl.Message(content=f"Word count: {word_count}").send()
        return
    ...
//}

コマンドを利用するための事前準備は次のとおりです。

 * コマンドのメタ情報を @<code>{CommandDict} の辞書として記述する
 * そのリストを @<code>{cl.context.emitter.set_commands} で送信して設定する

//emlist[Command 設定の実装][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    ...
    await cl.context.emitter.set_commands(
        [
            {
                "id": "Meow",
                "icon": "cat",
                "description": "Sends a meow message",
                "button": True,     # ボタンを表示するかどうか
                "persistent": True, # 実行後もコマンド選択を維持するか
            },
            {
                "id": "Word Count",
                "icon": "ruler",
                "description": "Counts the number of words in the message",
                "button": True,
                "persistent": True,
            },
        ]
    )
//}

@<code>{icon} の値は Lucide@<fn>{lucide} のアイコン名であり、コマンドのボタンに利用されます。
//footnote[lucide][@<href>{https://lucide.dev/}]


スターターと異なり、ユーザーはコマンドの実行時にメッセージを追加できます。
また、コマンドはチャット開始後も利用できるため、Skills@<fn>{skills} への入口として使うこともできます。
//footnote[skills][@<href>{https://code.claude.com/docs/ja/skills}]

=== チャット設定（ @<code>{cl.ChatSettings} ）

@<img>{chat_start} を見ると、コマンドボタンの隣に歯車のアイコンがあります。
これは Chat Settings と呼ばれる機能で、チャットの細かな設定を可能にします。

主に以下のような用途で使われます。

 * アシスタントが利用する LLM の選択やハイパーパラメータの指定
 * ユーザー固有の設定値（モードや機能の有効化・無効化など）の保存

Chat Settings は @<code>{cl.ChatSettings} を介して設定します。
入力ウィジェットにはトグルボタンやスライダーなど様々な種類が用意されており、用途に即して使い分けることができます。

//emlist[Chat Settings の実装][python]{
@cl.on_chat_start
async def on_chat_start() -> None:
    await cl.ChatSettings(
        [
            Select(
                id="Thinking mode",
                label="Thinking mode",
                values=["fast", "slow"],
                initial_index=0,
            ),
            Slider(
                id="Creativity",
                label="Creativity",
                initial=50,
                min=0,
                max=100,
            ),
            Switch(
                id="Enable feature X",
                label="Enable feature X",
                initial=False,
            ),
            Tags(
                id="Interests",
                label="Interests",
                values=["AI", "Machine Learning", "Data Science"],
                initial=["AI", "Data Science"],
            ),
            TextInput(
                id="Notes",
                label="Notes",
                placeholder="Enter your notes here",
            ),
        ]
    ).send()
    ...
//}

ユーザーが歯車のアイコンを押すと設定画面がポップアップされます。

//image[chat_setting][チャット設定画面][scale=0.8]{
//}

設定が更新されると、@<code>{@cl.on_settings_update} に登録した関数が呼び出されます。
設定をユーザーセッションに保存することで、アプリケーション側が設定内容に応じた処理を行えます。

//emlist[Chat Settings の利用][python]{
@cl.on_settings_update
async def setup_agent(settings: dict[str, Any]) -> None:
    cl.user_session.set("chat_settings", settings)
//}

== メッセージ

チャット内では、ユーザーとアシスタントのメッセージのやり取りが行われます。
ユーザーがメッセージを送信すると、@<code>{@cl.on_message} を修飾したコールバック関数が呼び出され、その中でアシスタントの処理と返答が行われます。

//image[command_meow_output][チャット画面][scale=0.8]{
//}

しかし、アシスタントの回答は単なるテキスト表示にとどまりません。たとえば：

 * ChatGPTでは、最終回答だけでなく、途中の思考やツールの実行内容も表示される
 * Claude Code や Codex などでは、ユーザーに Yes/No や選択肢を提示して操作を促す

そのため、アシスタントの意図を伝えるには、メッセージに関する UI 上の工夫が必要です。
これらの要求に幅広く対応するため、Chainlitではさまざまな機能を提供しています。
このセクションでは、その基本的な機能と実装方法を説明します。

=== Message（ @<code>{cl.Message} ）

@<code>{cl.Message} はユーザーとアシスタントがやり取りする最も基本的なメッセージです。
ユーザーの入力したメッセージや、アシスタントの出力した回答に対応します。

例えば、ユーザーの入力をそのまま出力する場合、実装は以下のようになります。

//emlist[@<code>{cl.Message} の利用][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    await cl.Message(content=f"Received: {message.content}").send()
//}

特に、ユーザーの入力メッセージは関数の引数を通じて与えられます。
メッセージは以下のように扱います。

 * @<code>{content} に文字列をセットする
 * @<code>{send()} で送信する

@<code>{send()} を呼び出すと、内部では以下のことが行われます。
@<fn>{messagebase-send}
//footnote[messagebase-send][詳細は @<code>{MessageBase.send()} の実装を参照してください。]

 * UI画面へのメッセージ送信（メッセージが表示される）
 * （もしあれば）データレイヤーにメッセージのレコードを追加
 * @<code>{cl.chat_context} にメッセージを追加

@<code>{cl.chat_context} は、チャット内でこれまでやり取りしたメッセージを @<code>{list[cl.Message]} として保存している変数です。
主な使い方は次のとおりです。

 * @<code>{cl.chat_context.get()} でメッセージ履歴を取得する
 * @<code>{cl.chat_context.to_openai()} で OpenAI 互換の形式に変換する（LLM の入力として利用できる）

なお、メッセージの内容は @<code>{update()} や @<code>{remove()} を用いて更新・削除できます（@<code>{send()} と同様に、内部で更新・削除が行われます）。

//emlist[@<code>{update()} 及び @<code>{remove()} の利用][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    msg = cl.Message(content="This message will be updated after 2 seconds.")
    await msg.send()

    await cl.sleep(2)

    msg.content = "This message will be removed after 2 seconds."
    await msg.update() # このタイミングでメッセージが更新される

    await cl.sleep(2)
    await msg.remove() # このタイミングでメッセージが削除される
//}

=== Step（ @<code>{cl.Step} ）

LLM アプリケーションでは、AI の思考の過程をユーザーに表示したい場面があります。例えば次のような情報です。

 * 検索ツールを使った
 * データベースを参照した
 * 中間計算を行った

Chainlit では、このような処理過程を可視化するための仕組みとして
@<code>{cl.Step} が用意されています。

使い方を学ぶため、以下の実装と対応する実行結果を見てみましょう。

//emlist[@<code>{cl.Step} の利用][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    async with cl.Step(
        name="Step started", # ステップに最初に表示される名前
        default_open=True,
    ) as step:
        await cl.sleep(1)
        ...

        async with cl.Step(         # ステップはネストすることができます。
            name="Tool call",       # 表示されるラベル
            type="tool",            # ステップの種類を表すタグ
            default_open=True,
            show_input="python",    # 入力のシンタックスハイライト
        ) as second:
            step.name = second.name

            second.input = "add(1, 2)"
            await step.update()
            await second.update()
            await cl.sleep(1)
            second.output = {"output": 3}

        step.name = "Step completed"
//}

//image[step][@<code>{cl.Step} の表示][scale=0.8]{
//}

このネストされたステップを実行すると、以下のように途中経過を表示しながら処理が進みます。

 1. 親ステップが @<code>{__aenter__()} し、@<code>{send()} により、「使用中：Step started」という文字列と共にステップが表示されます。
 1. 子ステップが @<code>{__aenter__()} し、ネストされた形で同様に表示されます。
 1. 子ステップの @<code>{input} と 親ステップの @<code>{name} が更新されます。 @<code>{update()} が呼び出されて画面に反映されます。
 1. 子ステップの @<code>{output} が更新され、子ステップが @<code>{__aexit__()} します。@<code>{__aexit__()} 内部で @<code>{update()} が呼び出されてUIに反映されます。
 1. 親ステップが @<code>{__aexit__()} します。表示は「使用済み：Step completed」となります。

コンテキストマネージャーを使用するため、やや分かりにくく感じるかもしれませんが、以下の点がポイントです。

 * @<code>{cl.Step} は @<code>{cl.Message} と同様のメソッド（send/update など）を持つ
 * @<code>{cl.Step} の内容はデータレイヤーの @<code>{steps} テーブルに永続化される
 * ただし、メッセージ履歴を管理する @<code>{cl.chat_context} には含まれない

このことから、@<code>{cl.Step} は「思考の過程」を表現するための機能であることが読み取れます。

=== Action（ @<code>{cl.Action} ）

@<code>{cl.Action} は、ユーザーがクリックできる「操作ボタン」です。
チャット UI にボタンを表示し、クリック時にサーバ側の処理を呼び出せます。

主なポイントは次のとおりです。

 * ボタンをクリックすることで、アプリ側のコールバックが呼び出される
 * そのコールバック内でメッセージを更新したり、別の処理を実行できる
 * データレイヤーには永続化されないため、画面リロードでボタンは消える

以下は、クリック回数を表示する単純なアクションの例です。

//emlist[@<code>{cl.Action} の利用][python]{
@cl.action_callback("count_clicks")
async def count_clicks(action: cl.Action) -> None:
    await action.remove()
    count_key = f"count:{action.forId}"

    count = cl.user_session.get(count_key, 0)
    count += 1
    cl.user_session.set(count_key, count)

    action.payload["count"] = count
    action.label = f"Clicked {count} times!"
    await action.send(for_id=action.forId)

@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    actions = [
        cl.Action(
            name="count_clicks",
            payload={"count": 0},
            label="Click me!",
            tooltip="This button will count the number of clicks.",
            icon="mouse-pointer-click",
        )
    ]
    await cl.Message(
        content="Let's click some buttons!", actions=actions
    ).send()
//}

 * @<code>{Action.name} がアクションの識別子となる
 * @<code>{cl.Message} に複数のアクションを設定できる
 * アクションのボタンが押された場合のコールバックは @<code>{@cl.action_callback("アクション名")} を用いて実装する

実行すると以下のようにクリックできるボタンが出現し、クリック回数が表示されます。

//image[action][@<code>{cl.Action} の表示][scale=0.8]{
//}

なお、@<code>{cl.Action} の情報はデータレイヤーに保存されません。そのため、以下の点に注意が必要です。

 * タブをリロードするとボタン自体が消える
 * 選択結果を残したい場合は、@<code>{cl.Message} や @<code>{cl.Step} に記録する必要がある

例えば、確認ダイアログとして使う場合は、ボタンに加えて「良ければYes、良くないならばNoと回答してください」といったメッセージも併記しておくと、再開時にユーザーが迷いにくくなります。

=== Element（ @<code>{cl.Element} ）

アシスタントの回答に画像や動画などの「テキスト以外の要素」を含めたい場合、@<code>{cl.Element} を利用します。

ポイント：

 * @<code>{cl.Element} はメッセージに付属させる形で追加する
 * 対応する要素には画像、動画、テーブルなどが含まれる

以下は簡単な例です。

//emlist[@<code>{cl.Element} の利用][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    elements = [
        cl.Text(
            name="text",
            display="inline",
            content="a text element with inline display.",
        ),
    ]
    await cl.Message(
        content="Here are some elements:", elements=elements
    ).send()
//}

Chainlitの@<code>{cl.Element}は画像や動画など、様々な要素に対応しています。
対応範囲については以下を参照してください。

 * @<href>{https://docs.chainlit.io/concepts/element}
 * @<href>{https://docs.chainlit.io/api-reference/elements/custom}

=== Ask User（ @<code>{cl.AskMessageBase} ）

@<code>{cl.Action} でボタン操作を促す代わりに、ユーザーにテキスト入力を求めたい場合があります。
そのようなときに使えるのが、@<code>{cl.AskUserMessage} および @<code>{cl.AskMessageBase} です。

ポイント：

 * ユーザーの入力（テキスト）を待ち受ける
 * 入力待ち中は @<code>{@cl.on_message} の処理が一時停止する
 * タイムアウト設定が可能（デフォルト 60 秒）

以下は、ユーザーに好きな色を尋ねる例です。

//emlist[@<code>{cl.AskUserMessage} の利用][python]{
@cl.on_message
async def on_message(message: cl.Message) -> None:
    ...
    res = await cl.AskUserMessage(
        content="What is your favorite color?",
    ).send()
    if res:
        await cl.Message(
            content=f"Your favorite color is {res['output']}"
        ).send()
//}

これを実行すると以下のようになります。

//image[ask][@<code>{cl.AskUserMessage}][scale=0.8]{
//}

動作のポイントは以下のとおりです。

 * ユーザーの入力待ちの間、@<code>{@cl.on_message} の処理は一時停止する
 * 待ち続けるとユーザー離脱時に処理が残る可能性があるため、タイムアウト設定が重要
 * デフォルトのタイムアウトは 60 秒

== まとめ

本章では、シンプルなウェブアプリケーションを例に、Chainlitの基本機能を概観しました。主なポイントは以下のとおりです。

 * ログイン機能：@<code>{@cl.password_auth_callback} によるパスワード認証を導入
 * チャット機能：
 ** スターター（@<code>{@cl.set_starters}）によるプリセットメッセージ
 ** アシスタント選択（@<code>{@cl.set_chat_profiles}）
 ** チャット再開（@<code>{@cl.on_chat_resume}）と履歴管理（@<code>{@cl.data_layer}）
 ** コマンド設定（@<code>{cl.context.emitter.set_commands}）
 ** チャット設定（@<code>{cl.ChatSettings}）
 * メッセージ機能：
 ** 基本メッセージ（@<code>{cl.Message}）
 ** 処理過程の可視化（@<code>{cl.Step}）
 ** 操作ボタン（@<code>{cl.Action}）
 ** テキスト以外の要素（@<code>{cl.Element}）
 ** ユーザー入力要求（@<code>{cl.AskUserMessage}）

これらの機能を組み合わせることで、Chainlitは LLM を活用したインタラクティブなアプリケーションを効率的に開発できるフレームワークになります。開発者は、認証・UI・データ管理の詳細を意識せず、ビジネスロジックに集中できます。
